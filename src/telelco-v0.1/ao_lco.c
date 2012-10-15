/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <ao.h>
#include <ao_lco.h>
#include <ao_event.h>
#include <ao_seven_segment.h>
#include <ao_quadrature.h>
#include <ao_lco_func.h>
#include <ao_radio_cmac.h>

#define DEBUG	1

#if DEBUG
static uint8_t	ao_lco_debug;
#define PRINTD(...) do { if (!ao_lco_debug) break; printf ("\r%5u %s: ", ao_tick_count, __func__); printf(__VA_ARGS__); flush(); } while(0)
#else
#define PRINTD(...) 
#endif

#define AO_LCO_PAD_DIGIT	0
#define AO_LCO_BOX_DIGIT_1	1
#define AO_LCO_BOX_DIGIT_10	2

static uint8_t	ao_lco_min_box, ao_lco_max_box;
static uint8_t	ao_lco_mutex;
static uint8_t	ao_lco_pad;
static uint8_t	ao_lco_box;
static uint8_t	ao_lco_armed;
static uint8_t	ao_lco_firing;
static uint8_t	ao_lco_valid;
static uint8_t	ao_lco_got_channels;
static uint16_t	ao_lco_tick_offset;

static struct ao_pad_query	ao_pad_query;

static void
ao_lco_set_pad(void)
{
	ao_seven_segment_set(AO_LCO_PAD_DIGIT, ao_lco_pad + 1);
}

static void
ao_lco_set_box(void)
{
	ao_seven_segment_set(AO_LCO_BOX_DIGIT_1, ao_lco_box % 10);
	ao_seven_segment_set(AO_LCO_BOX_DIGIT_10, ao_lco_box / 10);
}

#define MASK_SIZE(n)	(((n) + 7) >> 3)
#define MASK_ID(n)	((n) >> 3)
#define MASK_SHIFT(n)	((n) & 7)

static uint8_t	ao_lco_box_mask[MASK_SIZE(AO_PAD_MAX_BOXES)];

static uint8_t
ao_lco_box_present(uint8_t box)
{
	if (box >= AO_PAD_MAX_BOXES)
		return 0;
	return (ao_lco_box_mask[MASK_ID(box)] >> MASK_SHIFT(box)) & 1;
}

static uint8_t
ao_lco_pad_present(uint8_t pad)
{
	if (!ao_lco_got_channels || !ao_pad_query.channels)
		return pad == 0;
	if (pad >= AO_PAD_MAX_CHANNELS)
		return 0;
	return (ao_pad_query.channels >> pad) & 1;
}

static uint8_t
ao_lco_pad_first(void)
{
	uint8_t	pad;

	for (pad = 0; pad < AO_PAD_MAX_CHANNELS; pad++)
		if (ao_lco_pad_present(pad))
			return pad;
	return 0;
}

static void
ao_lco_input(void)
{
	static struct ao_event	event;
	int8_t	dir, new_box, new_pad;

	ao_beep_for(AO_BEEP_MID, AO_MS_TO_TICKS(200));
	ao_lco_set_pad();
	ao_lco_set_box();
	for (;;) {
		ao_event_get(&event);
		PRINTD("event type %d unit %d value %d\n",
		       event.type, event.unit, event.value);
		switch (event.type) {
		case AO_EVENT_QUADRATURE:
			switch (event.unit) {
			case AO_QUADRATURE_PAD:
				if (!ao_lco_armed) {
					if (event.value == ao_lco_pad)
						break;
					dir = ((int8_t) event.value - (int8_t) ao_lco_pad) > 0 ? 1 : -1;
					new_pad = event.value;
					while (!ao_lco_pad_present(new_pad)) {
						new_pad += dir;
						if (new_pad > AO_PAD_MAX_CHANNELS)
							new_pad = 0;
						else if (new_pad < 0)
							new_pad = AO_PAD_MAX_CHANNELS - 1;
						if (new_pad == ao_lco_pad)
							break;
					}
					if (new_pad != ao_lco_pad) {
						ao_lco_pad = new_pad;
						ao_quadrature_count[AO_QUADRATURE_PAD] = ao_lco_pad;
						ao_lco_set_pad();
					}
				}
				break;
			case AO_QUADRATURE_BOX:
				if (!ao_lco_armed) {
					if (event.value == ao_lco_box)
						break;
					dir = ((int8_t) event.value - (int8_t) ao_lco_box) > 0 ? 1 : -1;
					new_box = event.value;
					while (!ao_lco_box_present(new_box)) {
						new_box += dir;
						if (new_box > ao_lco_max_box)
							new_box = ao_lco_min_box;
						else if (new_box < ao_lco_min_box)
							new_box = ao_lco_min_box;
						if (new_box == ao_lco_box)
							break;
					}
					ao_quadrature_count[AO_QUADRATURE_PAD] = new_box;
					if (ao_lco_box != new_box) {
						ao_lco_box = new_box;
						ao_lco_got_channels = 0;
						ao_lco_set_box();
					}
				}
				break;
			}
			break;
		case AO_EVENT_BUTTON:
			switch (event.unit) {
			case AO_BUTTON_ARM:
				ao_lco_armed = event.value;
				PRINTD("Armed %d\n", ao_lco_armed);
				ao_wakeup(&ao_lco_armed);
				break;
			case AO_BUTTON_FIRE:
				if (ao_lco_armed) {
					ao_lco_firing = event.value;
					PRINTD("Firing %d\n", ao_lco_firing);
					ao_wakeup(&ao_lco_armed);
				}
				break;
			}
			break;
		}
	}
}

static AO_LED_TYPE	continuity_led[AO_LED_CONTINUITY_NUM] = {
#ifdef AO_LED_CONTINUITY_0
	AO_LED_CONTINUITY_0,
#endif
#ifdef AO_LED_CONTINUITY_1
	AO_LED_CONTINUITY_1,
#endif
#ifdef AO_LED_CONTINUITY_2
	AO_LED_CONTINUITY_2,
#endif
#ifdef AO_LED_CONTINUITY_3
	AO_LED_CONTINUITY_3,
#endif
#ifdef AO_LED_CONTINUITY_4
	AO_LED_CONTINUITY_4,
#endif
#ifdef AO_LED_CONTINUITY_5
	AO_LED_CONTINUITY_5,
#endif
#ifdef AO_LED_CONTINUITY_6
	AO_LED_CONTINUITY_6,
#endif
#ifdef AO_LED_CONTINUITY_7
	AO_LED_CONTINUITY_7,
#endif
};

static void
ao_lco_update(void)
{
	int8_t			r;
	uint8_t			c;

	r = ao_lco_query(ao_lco_box, &ao_pad_query, &ao_lco_tick_offset);
	if (r == AO_RADIO_CMAC_OK) {
		c = ao_lco_got_channels;
		ao_lco_got_channels = 1;
		ao_lco_valid = 1;
		if (!c) {
			ao_lco_pad = ao_lco_pad_first();
			ao_lco_set_pad();
		}
	} else
		ao_lco_valid = 0;

#if 0
	PRINTD("lco_query success arm_status %d i0 %d i1 %d i2 %d i3 %d\n",
	       query.arm_status,
	       query.igniter_status[0],
	       query.igniter_status[1],
	       query.igniter_status[2],
	       query.igniter_status[3]);
#endif

	ao_wakeup(&ao_pad_query);
}

static void
ao_lco_box_set_present(uint8_t box)
{
	if (box >= AO_PAD_MAX_BOXES)
		return;
	ao_lco_box_mask[MASK_ID(box)] |= 1 << MASK_SHIFT(box);
}

static void
ao_lco_search(void)
{
	uint16_t	tick_offset;
	int8_t		r;

	ao_lco_min_box = 0xff;
	ao_lco_max_box = 0x00;
	for (ao_lco_box = 0; ao_lco_box < AO_PAD_MAX_BOXES; ao_lco_box++) {
		if ((ao_lco_box % 10) == 0)
			ao_lco_set_box();
		r = ao_lco_query(ao_lco_box, &ao_pad_query, &ao_lco_tick_offset);
		if (r == AO_RADIO_CMAC_OK) {
			if (ao_lco_box < ao_lco_min_box)
				ao_lco_min_box = ao_lco_box;
			if (ao_lco_box > ao_lco_max_box)
				ao_lco_max_box = ao_lco_box;
			ao_lco_box_set_present(ao_lco_box);
		}
	}
	if (ao_lco_min_box <= ao_lco_max_box)
		ao_lco_box = ao_lco_min_box;
	else
		ao_lco_min_box = ao_lco_max_box = ao_lco_box = 0;
	ao_lco_valid = 0;
	ao_lco_got_channels = 0;
	ao_lco_pad = 0;
}

static void
ao_lco_igniter_status(void)
{
	uint8_t		c;
	uint16_t	delay;

	for (;;) {
//		ao_alarm(delay);
		ao_sleep(&ao_pad_query);
//		ao_clear_alarm();
		if (!ao_lco_valid) {
			ao_led_on(AO_LED_RED);
			ao_led_off(AO_LED_GREEN);
			continue;
		}
		PRINTD("RSSI %d\n", ao_radio_cmac_rssi);
		if (ao_radio_cmac_rssi < -90)
			ao_led_on(AO_LED_RED|AO_LED_GREEN);
		else {
			ao_led_on(AO_LED_GREEN);
			ao_led_off(AO_LED_RED);
		}
		if (ao_pad_query.arm_status)
			ao_led_on(AO_LED_REMOTE_ARM);
		else
			ao_led_off(AO_LED_REMOTE_ARM);
		for (c = 0; c < AO_LED_CONTINUITY_NUM; c++) {
			uint8_t	status;

			if (ao_pad_query.channels & (1 << c))
				status = ao_pad_query.igniter_status[c];
			else
				status = AO_PAD_IGNITER_STATUS_NO_IGNITER_RELAY_OPEN;
			if (status == AO_PAD_IGNITER_STATUS_GOOD_IGNITER_RELAY_OPEN)
				ao_led_on(continuity_led[c]);
			else
				ao_led_off(continuity_led[c]);
		}
	}
}

static void
ao_lco_arm_warn(void)
{
	for (;;) {
		while (!ao_lco_armed)
			ao_sleep(&ao_lco_armed);
		ao_beep_for(AO_BEEP_MID, AO_MS_TO_TICKS(200));
		ao_delay(AO_MS_TO_TICKS(200));
	}
}

static struct ao_task ao_lco_input_task;
static struct ao_task ao_lco_monitor_task;
static struct ao_task ao_lco_arm_warn_task;
static struct ao_task ao_lco_igniter_status_task;

static void
ao_lco_monitor(void)
{
	uint16_t		delay;

	ao_lco_search();
	ao_add_task(&ao_lco_input_task, ao_lco_input, "lco input");
	ao_add_task(&ao_lco_arm_warn_task, ao_lco_arm_warn, "lco arm warn");
	ao_add_task(&ao_lco_igniter_status_task, ao_lco_igniter_status, "lco igniter status");
	for (;;) {
		PRINTD("monitor armed %d firing %d offset %d\n",
		       ao_lco_armed, ao_lco_firing, ao_lco_tick_offset);

		if (ao_lco_armed && ao_lco_firing) {
			PRINTD("Firing box %d pad %d: valid %d\n",
			       ao_lco_box, ao_lco_pad, ao_lco_valid);
			if (!ao_lco_valid)
				ao_lco_update();
			if (ao_lco_valid)
				ao_lco_ignite(ao_lco_box, 1 << ao_lco_pad, ao_lco_tick_offset);
		} else if (ao_lco_armed) {
			PRINTD("Arming box %d pad %d\n",
			       ao_lco_box, ao_lco_pad);
			if (!ao_lco_valid)
				ao_lco_update();
			ao_lco_arm(ao_lco_box, 1 << ao_lco_pad, ao_lco_tick_offset);
			ao_lco_update();
		} else {
			ao_lco_update();
		}
		if (ao_lco_armed && ao_lco_firing)
			delay = AO_MS_TO_TICKS(100);
		else
			delay = AO_SEC_TO_TICKS(1);
		ao_alarm(delay);
		ao_sleep(&ao_lco_armed);
		ao_clear_alarm();
	}
}

#if DEBUG
void
ao_lco_set_debug(void)
{
	ao_cmd_decimal();
	if (ao_cmd_status == ao_cmd_success)
		ao_lco_debug = ao_cmd_lex_i != 0;
}

__code struct ao_cmds ao_lco_cmds[] = {
	{ ao_lco_set_debug,	"D <0 off, 1 on>\0Debug" },
	{ 0, NULL }
};
#endif

void
ao_lco_init(void)
{
	ao_add_task(&ao_lco_monitor_task, ao_lco_monitor, "lco monitor");
#if DEBUG
	ao_cmd_register(&ao_lco_cmds[0]);
#endif
}

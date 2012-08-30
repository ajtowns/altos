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

#if 1
#define PRINTD(...) do { printf ("\r%5u %s: ", ao_tick_count, __func__); printf(__VA_ARGS__); flush(); } while(0)
#else
#define PRINTD(...) 
#endif

#define AO_LCO_PAD_DIGIT	0
#define AO_LCO_BOX_DIGIT_1	1
#define AO_LCO_BOX_DIGIT_10	2

#define AO_NUM_BOX		10

static uint8_t	ao_lco_mutex;
static uint8_t	ao_lco_pad;
static uint8_t	ao_lco_box;
static uint8_t	ao_lco_armed;
static uint8_t	ao_lco_firing;
static uint8_t	ao_lco_valid;

static void
ao_lco_set_pad(void)
{
	ao_seven_segment_set(AO_LCO_PAD_DIGIT, ao_lco_pad);
}

static void
ao_lco_set_box(void)
{
	ao_seven_segment_set(AO_LCO_BOX_DIGIT_1, ao_lco_box % 10);
	ao_seven_segment_set(AO_LCO_BOX_DIGIT_10, ao_lco_box / 10);
}

static void
ao_lco_input(void)
{
	static struct ao_event	event;

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
					ao_lco_pad = event.value & 3;
					ao_lco_valid = 0;
					ao_quadrature_count[AO_QUADRATURE_PAD] = ao_lco_pad;
					ao_lco_set_pad();
				}
				break;
			case AO_QUADRATURE_BOX:
				if (!ao_lco_armed) {
					ao_lco_box = event.value;
					ao_lco_valid = 0;
					while (ao_lco_box >= AO_NUM_BOX)
						ao_lco_box -= AO_NUM_BOX;
					while (ao_lco_box < 0)
						ao_lco_box += AO_NUM_BOX;
					ao_quadrature_count[AO_QUADRATURE_PAD] = ao_lco_box;
					ao_lco_set_box();
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

static uint16_t	ao_lco_tick_offset;

static void
ao_lco_update(void)
{
	int8_t			r;
	uint8_t			c;
	struct ao_pad_query	query;

	r = ao_lco_query(ao_lco_box, &query, &ao_lco_tick_offset);
	if (r != AO_RADIO_CMAC_OK) {
		PRINTD("lco_query return %d\n", r);
		return;
	}

#if 0
	PRINTD("lco_query success arm_status %d i0 %d i1 %d i2 %d i3 %d\n",
	       query.arm_status,
	       query.igniter_status[0],
	       query.igniter_status[1],
	       query.igniter_status[2],
	       query.igniter_status[3]);
#endif

	ao_lco_valid = 1;
	if (query.arm_status)
		ao_led_on(AO_LED_REMOTE_ARM);
	else
		ao_led_off(AO_LED_REMOTE_ARM);
	for (c = 0; c < AO_LED_CONTINUITY_NUM; c++) {
		uint8_t	status;

		if (query.channels & (1 << c))
			status = query.igniter_status[c];
		else
			status = AO_PAD_IGNITER_STATUS_NO_IGNITER_RELAY_OPEN;
		if (status == AO_PAD_IGNITER_STATUS_GOOD_IGNITER_RELAY_OPEN)
			ao_led_on(continuity_led[c]);
		else
			ao_led_off(continuity_led[c]);
	}
}

static void
ao_lco_monitor(void)
{
	uint16_t		delay;

	for (;;) {
		if (ao_lco_armed && ao_lco_firing) {
			PRINTD("Firing box %d pad %d: valid %d\n",
			       ao_lco_box, ao_lco_pad, ao_lco_valid);
			if (!ao_lco_valid)
				ao_lco_update();
			if (ao_lco_valid)
				ao_lco_ignite(ao_lco_box, ao_lco_pad, ao_lco_tick_offset);
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

void
ao_lco_init(void)
{
	ao_add_task(&ao_lco_input_task, ao_lco_input, "lco input");
	ao_add_task(&ao_lco_monitor_task, ao_lco_monitor, "lco monitor");
	ao_add_task(&ao_lco_arm_warn_task, ao_lco_arm_warn, "lco arm warn");
}

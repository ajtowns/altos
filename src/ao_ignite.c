/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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

#include "ao.h"

#if IGNITE_ON_P2
#define AO_IGNITER_DROGUE	P2_3
#define AO_IGNITER_MAIN		P2_4
#define AO_IGNITER_DIR		P2DIR
#define AO_IGNITER_DROGUE_BIT	(1 << 3)
#define AO_IGNITER_MAIN_BIT	(1 << 4)
#endif

#if IGNITE_ON_P0
#define AO_IGNITER_DROGUE	P0_5
#define AO_IGNITER_MAIN		P0_4
#define AO_IGNITER_DIR		P0DIR
#define AO_IGNITER_DROGUE_BIT	(1 << 5)
#define AO_IGNITER_MAIN_BIT	(1 << 4)
#endif

/* test these values with real igniters */
#define AO_IGNITER_OPEN		1000
#define AO_IGNITER_CLOSED	7000
#define AO_IGNITER_FIRE_TIME	AO_MS_TO_TICKS(50)
#define AO_IGNITER_CHARGE_TIME	AO_MS_TO_TICKS(2000)

struct ao_ignition {
	uint8_t	request;
	uint8_t fired;
	uint8_t firing;
};

__xdata struct ao_ignition ao_ignition[2];

void
ao_ignite(enum ao_igniter igniter) __critical
{
	ao_ignition[igniter].request = 1;
	ao_wakeup(&ao_ignition);
}

enum ao_igniter_status
ao_igniter_status(enum ao_igniter igniter)
{
	__xdata struct ao_adc adc;
	__pdata int16_t value;
	__pdata uint8_t request, firing, fired;

	__critical {
		ao_adc_get(&adc);
		request = ao_ignition[igniter].request;
		fired = ao_ignition[igniter].fired;
		firing = ao_ignition[igniter].firing;
	}
	if (firing || (request && !fired))
		return ao_igniter_active;

	value = (AO_IGNITER_CLOSED>>1);
	switch (igniter) {
	case ao_igniter_drogue:
		value = adc.sense_d;
		break;
	case ao_igniter_main:
		value = adc.sense_m;
		break;
	}
	if (value < AO_IGNITER_OPEN)
		return ao_igniter_open;
	else if (value > AO_IGNITER_CLOSED)
		return ao_igniter_ready;
	else
		return ao_igniter_unknown;
}

void
ao_igniter_fire(enum ao_igniter igniter) __critical
{
	ao_ignition[igniter].firing = 1;
	switch(ao_config.ignite_mode) {
	case AO_IGNITE_MODE_DUAL:
		switch (igniter) {
		case ao_igniter_drogue:
			AO_IGNITER_DROGUE = 1;
			ao_delay(AO_IGNITER_FIRE_TIME);
			AO_IGNITER_DROGUE = 0;
			break;
		case ao_igniter_main:
			AO_IGNITER_MAIN = 1;
			ao_delay(AO_IGNITER_FIRE_TIME);
			AO_IGNITER_MAIN = 0;
			break;
		}
		break;
	case AO_IGNITE_MODE_APOGEE:
		switch (igniter) {
		case ao_igniter_drogue:
			AO_IGNITER_DROGUE = 1;
			ao_delay(AO_IGNITER_FIRE_TIME);
			AO_IGNITER_DROGUE = 0;
			ao_delay(AO_IGNITER_CHARGE_TIME);
			AO_IGNITER_MAIN = 1;
			ao_delay(AO_IGNITER_FIRE_TIME);
			AO_IGNITER_MAIN = 0;
			break;
		}
		break;
	case AO_IGNITE_MODE_MAIN:
		switch (igniter) {
		case ao_igniter_main:
			AO_IGNITER_DROGUE = 1;
			ao_delay(AO_IGNITER_FIRE_TIME);
			AO_IGNITER_DROGUE = 0;
			ao_delay(AO_IGNITER_CHARGE_TIME);
			AO_IGNITER_MAIN = 1;
			ao_delay(AO_IGNITER_FIRE_TIME);
			AO_IGNITER_MAIN = 0;
			break;
		}
		break;
	}
	ao_ignition[igniter].firing = 0;
}

void
ao_igniter(void)
{
	__xdata enum ao_ignter igniter;

	ao_config_get();
	for (;;) {
		ao_sleep(&ao_ignition);
		for (igniter = ao_igniter_drogue; igniter <= ao_igniter_main; igniter++) {
			if (ao_ignition[igniter].request && !ao_ignition[igniter].fired) {
				if (igniter == ao_igniter_drogue && ao_config.apogee_delay)
					ao_delay(AO_SEC_TO_TICKS(ao_config.apogee_delay));

				ao_igniter_fire(igniter);
				ao_delay(AO_IGNITER_CHARGE_TIME);
				ao_ignition[igniter].fired = 1;
			}
		}
	}
}

void
ao_ignite_manual(void)
{
	ao_cmd_white();
	if (!ao_match_word("DoIt"))
		return;
	ao_cmd_white();
	if (ao_cmd_lex_c == 'm') {
		if(ao_match_word("main"))
			ao_igniter_fire(ao_igniter_main);
	} else {
		if(ao_match_word("drogue"))
			ao_igniter_fire(ao_igniter_drogue);
	}
}

static __code char * __code igniter_status_names[] = {
	"unknown", "ready", "active", "open"
};

void
ao_ignite_print_status(enum ao_igniter igniter, __code char *name) __reentrant
{
	enum ao_igniter_status status = ao_igniter_status(igniter);
	printf("Igniter: %6s Status: %s\n",
	       name,
	       igniter_status_names[status]);
}

void
ao_ignite_test(void)
{
	ao_ignite_print_status(ao_igniter_drogue, "drogue");
	ao_ignite_print_status(ao_igniter_main, "main");
}

__code struct ao_cmds ao_ignite_cmds[] = {
	{ ao_ignite_manual,	"i <key> {main|drogue}\0Fire igniter. <key> is doit with D&I" },
	{ ao_ignite_test,	"t\0Test igniter" },
	{ 0,	NULL },
};

__xdata struct ao_task ao_igniter_task;

void
ao_ignite_set_pins(void)
{
	AO_IGNITER_DROGUE = 0;
	AO_IGNITER_MAIN = 0;
	AO_IGNITER_DIR |= AO_IGNITER_DROGUE_BIT | AO_IGNITER_MAIN_BIT;
}

void
ao_igniter_init(void)
{
	ao_ignite_set_pins();
	ao_cmd_register(&ao_ignite_cmds[0]);
	ao_add_task(&ao_igniter_task, ao_igniter, "igniter");
}

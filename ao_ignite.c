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

#define AO_IGNITER_DROGUE	P2_3
#define AO_IGNITER_MAIN		P2_4

/* XXX test these values with real igniters */
#define AO_IGNITER_OPEN		100
#define AO_IGNITER_CLOSED	20000
#define AO_IGNITER_FIRE_TIME	AO_MS_TO_TICKS(50)
#define AO_IGNITER_CHARGE_TIME	AO_MS_TO_TICKS(200)

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
	ao_wakeup(&ao_ignition[0]);
}

enum ao_igniter_status
ao_igniter_status(enum ao_igniter igniter)
{
	__xdata struct ao_adc adc;
	__xdata int16_t value;
	__xdata uint8_t request, firing, fired;

	__critical {
		ao_adc_sleep();
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
	ao_ignition[igniter].firing = 0;
}

void
ao_igniter(void)
{
	__xdata enum ao_ignter igniter;
	__xdata enum ao_igniter_status status;

	for (;;) {
		ao_sleep(&ao_ignition);
		for (igniter = ao_igniter_drogue; igniter <= ao_igniter_main; igniter++) {
			if (ao_ignition[igniter].request && !ao_ignition[igniter].fired) {
				ao_igniter_fire(igniter);
				ao_delay(AO_IGNITER_CHARGE_TIME);
				status = ao_igniter_status(igniter);
				if (status == ao_igniter_open)
					ao_ignition[igniter].fired = 1;
			}
		}
	}
}

__xdata struct ao_task ao_igniter_task;

void
ao_igniter_init(void)
{
	ao_add_task(&ao_igniter_task, ao_igniter, "igniter");
}

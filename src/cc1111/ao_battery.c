/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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
static __data union {
	uint8_t	d[2];
	int16_t	v;
} ao_battery_value;

void
ao_battery_isr(void) ao_arch_interrupt(1)
{
	ao_battery_value.d[0] = ADCL;
	ao_battery_value.d[1] = ADCH;
	ao_wakeup(DATA_TO_XDATA(&ao_battery_value));
}

uint16_t
ao_battery_get(void) 
{
	ao_arch_critical(
		ADCCON3 = ADCCON3_EREF_VDD | ADCCON3_EDIV_512 | BATTERY_PIN;
		ao_sleep(DATA_TO_XDATA(&ao_battery_value));
		);
	return (uint16_t) ((int32_t) ao_battery_value.v * (int32_t) 4950 >> 15);
}

static void
ao_battery_show(void)
{
	printf("Battery: %u mV\n", ao_battery_get());
}

__code struct ao_cmds ao_battery_cmds[] = {
	{ ao_battery_show,	"B\0Show battery voltage" },
	{ 0, NULL },
};

void
ao_battery_init(void)
{
	ADCCFG = (1 << BATTERY_PIN);
	ADCIF = 0;
	IEN0 |= IEN0_ADCIE;
	ao_cmd_register(&ao_battery_cmds[0]);
}

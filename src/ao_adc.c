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

volatile __xdata struct ao_adc	ao_adc_ring[AO_ADC_RING];
volatile __data uint8_t		ao_adc_head;

void
ao_adc_poll(void)
{
	ADCCON3 = ADCCON3_EREF_VDD | ADCCON3_EDIV_512 | 0;
}

void
ao_adc_sleep(void)
{
	ao_sleep(&ao_adc_ring);
}

void
ao_adc_get(__xdata struct ao_adc *packet)
{
	uint8_t	i = ao_adc_ring_prev(ao_adc_head);
	memcpy(packet, &ao_adc_ring[i], sizeof (struct ao_adc));
}

void
ao_adc_isr(void) interrupt 1
{
	uint8_t sequence;
	uint8_t	__xdata *a;

	sequence = (ADCCON2 & ADCCON2_SCH_MASK) >> ADCCON2_SCH_SHIFT;
	if (sequence == ADCCON3_ECH_TEMP)
		sequence = 2;
	a = (uint8_t __xdata *) (&ao_adc_ring[ao_adc_head].accel + sequence);
	a[0] = ADCL;
	a[1] = ADCH;
	if (sequence < 5) {
		/* start next channel conversion */
		/* v0.2 replaces external temp sensor with internal one */
		if (sequence == 1)
			ADCCON3 = ADCCON3_EREF_1_25 | ADCCON3_EDIV_512 | ADCCON3_ECH_TEMP;
		else
			ADCCON3 = ADCCON3_EREF_VDD | ADCCON3_EDIV_512 | (sequence + 1);
	} else {
		/* record this conversion series */
		ao_adc_ring[ao_adc_head].tick = ao_time();
		ao_adc_head = ao_adc_ring_next(ao_adc_head);
		ao_wakeup(ao_adc_ring);
	}
}

static void
ao_adc_dump(void) __reentrant
{
	static __xdata struct ao_adc	packet;
	ao_adc_get(&packet);
	printf("tick: %5u accel: %5d pres: %5d temp: %5d batt: %5d drogue: %5d main: %5d\n",
	       packet.tick, packet.accel, packet.pres, packet.temp,
	       packet.v_batt, packet.sense_d, packet.sense_m);
}

__code struct ao_cmds ao_adc_cmds[] = {
	{ 'a',	ao_adc_dump,	"a                                  Display current ADC values" },
	{ 0,	ao_adc_dump, NULL },
};

void
ao_adc_init(void)
{
	ADCCFG = ((1 << 0) |	/* acceleration */
		  (1 << 1) |	/* pressure */
/*		  (1 << 2) |	  v0.1 temperature */
		  (1 << 3) |	/* battery voltage */
		  (1 << 4) |	/* drogue sense */
		  (1 << 5));	/* main sense */

	/* enable interrupts */
	ADCIF = 0;
	IEN0 |= IEN0_ADCIE;
	ao_cmd_register(&ao_adc_cmds[0]);
}

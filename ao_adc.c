/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

__xdata struct ao_adc	ao_adc_ring[ADC_RING];
__data uint8_t		ao_adc_head;

void ao_adc_isr(void) interrupt 1
{
	uint8_t sequence;
	uint8_t	__xdata *a;
	
	sequence = (ADCCON2 & ADCCON2_SCH_MASK) >> ADCCON2_SCH_SHIFT;
	a = (uint8_t __xdata *) (&ao_adc_ring[ao_adc_head].accel + sequence);
	a[0] = ADCL;
	a[1] = ADCH;
	if (sequence < 5) {
		/* start next channel conversion */
		ADCCON3 = ADCCON3_EREF_VDD | ADCCON3_EDIV_512 | (sequence + 1);
	} else {
		/* record this conversion series */
		ao_adc_ring[ao_adc_head].tick = ao_time;
		ao_adc_head++;
		if (ao_adc_head == ADC_RING)
			ao_adc_head = 0;
		ao_wakeup(ao_adc_ring);
	}
}

void ao_adc_init(void)
{
	ADCCFG = ((1 << 0) |	/* acceleration */
		  (1 << 1) |	/* pressure */
		  (1 << 2) |	/* temperature */
		  (1 << 3) |	/* battery voltage */
		  (1 << 4) |	/* drogue sense */
		  (1 << 5));	/* main sense */
	
	/* enable interrupts */
	ADCIF = 0;
	IEN0 |= IEN0_ADCIE;
}

void ao_adc_poll(void)
{
	ADCCON3 = ADCCON3_EREF_VDD | ADCCON3_EDIV_512 | 0;
}

void ao_adc_get(__xdata struct ao_adc *packet)
{
	uint8_t	i = ao_adc_head;
	if (i == 0)
		i = ADC_RING;
	i--;
	memcpy(packet, &ao_adc_ring[i], sizeof (struct ao_adc));
}


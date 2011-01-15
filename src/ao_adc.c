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
#include "ao_pins.h"

volatile __xdata struct ao_adc	ao_adc_ring[AO_ADC_RING];
#if HAS_ACCEL_REF
volatile __xdata uint16_t 	ao_accel_ref[AO_ADC_RING];
#endif
volatile __data uint8_t		ao_adc_head;

void
ao_adc_poll(void)
{
#if HAS_ACCEL_REF
	ADCCON3 = ADCCON3_EREF_VDD | ADCCON3_EDIV_512 | 2;
#else
	ADCCON3 = ADCCON3_EREF_VDD | ADCCON3_EDIV_512 | 0;
#endif
}

void
ao_adc_get(__xdata struct ao_adc *packet)
{
	uint8_t	i = ao_adc_ring_prev(ao_flight_adc);
	memcpy(packet, &ao_adc_ring[i], sizeof (struct ao_adc));
}

void
ao_adc_isr(void) __interrupt 1
{
	uint8_t sequence;
	uint8_t	__xdata *a;

	sequence = (ADCCON2 & ADCCON2_SCH_MASK) >> ADCCON2_SCH_SHIFT;
#if HAS_ACCEL_REF
	if (sequence == 2) {
		a = (uint8_t __xdata *) (&ao_accel_ref[ao_adc_head]);
		sequence = 0;
	} else
#endif
	{
		if (sequence == ADCCON3_ECH_TEMP)
			sequence = 2;
		a = (uint8_t __xdata *) (&ao_adc_ring[ao_adc_head].accel + sequence);
		sequence++;
	}
	a[0] = ADCL;
	a[1] = ADCH;
	if (sequence < 6) {
#if HAS_EXTERNAL_TEMP == 0
		/* start next channel conversion */
		/* v0.2 replaces external temp sensor with internal one */
		if (sequence == 2)
			ADCCON3 = ADCCON3_EREF_1_25 | ADCCON3_EDIV_512 | ADCCON3_ECH_TEMP;
		else
#endif
			ADCCON3 = ADCCON3_EREF_VDD | ADCCON3_EDIV_512 | sequence;
	} else {
		/* record this conversion series */
		ao_adc_ring[ao_adc_head].tick = ao_time();
		ao_adc_head = ao_adc_ring_next(ao_adc_head);
		ao_wakeup(DATA_TO_XDATA(&ao_adc_head));
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
#if HAS_EXTERNAL_TEMP
		  (1 << 2) |	/* v0.1 temperature */
#endif
		  (1 << 3) |	/* battery voltage */
		  (1 << 4) |	/* drogue sense */
		  (1 << 5));	/* main sense */

	/* enable interrupts */
	ADCIF = 0;
	IEN0 |= IEN0_ADCIE;
	ao_cmd_register(&ao_adc_cmds[0]);
}

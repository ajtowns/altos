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

volatile __xdata struct ao_data	ao_data_ring[AO_DATA_RING];
volatile __data uint8_t		ao_data_head;
#if (AO_DATA_ALL & ~(AO_DATA_ADC))
volatile __data uint8_t		ao_data_present;
#endif

#ifdef TELENANO_V_0_1
# define AO_ADC_FIRST_PIN	1
#endif

#if HAS_ACCEL_REF
# define AO_ADC_FIRST_PIN	2
#endif

#ifndef AO_ADC_FIRST_PIN
# define AO_ADC_FIRST_PIN	0
#endif

void
ao_adc_poll(void)
{
	ADCCON3 = ADCCON3_EREF_VDD | ADCCON3_EDIV_512 | AO_ADC_FIRST_PIN;
}

void
ao_data_get(__xdata struct ao_data *packet)
{
#if HAS_FLIGHT
	uint8_t	i = ao_data_ring_prev(ao_sample_data);
#else
	uint8_t	i = ao_data_ring_prev(ao_data_head);
#endif
	ao_xmemcpy(packet, (void __xdata *) &ao_data_ring[i], sizeof (struct ao_data));
}

void
ao_adc_isr(void) __interrupt 1
{
	uint8_t sequence;
	uint8_t	__xdata *a;

	sequence = (ADCCON2 & ADCCON2_SCH_MASK) >> ADCCON2_SCH_SHIFT;
#if TELEMETRUM_V_0_1 || TELEMETRUM_V_0_2 || TELEMETRUM_V_1_0 || TELEMETRUM_V_1_1 || TELEMETRUM_V_1_2 || TELELAUNCH_V_0_1 || TELEBALLOON_V_1_1
	/* TeleMetrum readings */
#if HAS_ACCEL_REF
	if (sequence == 2) {
		a = (uint8_t __xdata *) (&ao_data_ring[ao_data_head].adc.accel_ref);
		sequence = 0;
	} else
#endif
	{
		if (sequence == ADCCON3_ECH_TEMP)
			sequence = 2;
		a = (uint8_t __xdata *) (&ao_data_ring[ao_data_head].adc.accel + sequence);
		sequence++;
	}
#define GOT_ADC
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
		return;
	}
#endif

#if TELEMINI_V_1_0 || TELENANO_V_0_1
	/* TeleMini readings */
	a = (uint8_t __xdata *) (&ao_data_ring[ao_data_head].adc.pres);
#if TELEMINI_V_1_0
	switch (sequence) {
	case 0:
		/* pressure */
		a += 0;
		sequence = ADCCON3_EREF_VDD | ADCCON3_EDIV_512 | 1;
		break;
	case 1:
		/* drogue sense */
		a += 6;
		sequence = ADCCON3_EREF_VDD | ADCCON3_EDIV_512 | 2;
		break;
	case 2:
		/* main sense */
		a += 8;
		sequence = ADCCON3_EREF_VDD | ADCCON3_EDIV_512 | 3;
		break;
	case 3:
		/* battery */
		a += 4;
		sequence = ADCCON3_EREF_1_25 | ADCCON3_EDIV_512 | ADCCON3_ECH_TEMP;
		break;
	case ADCCON3_ECH_TEMP:
		a += 2;
		sequence = 0;
		break;
	}
#define GOT_ADC
#endif
#ifdef TELENANO_V_0_1
	switch (sequence) {
	case 1:
		/* pressure */
		a += 0;
		sequence = ADCCON3_EREF_VDD | ADCCON3_EDIV_512 | 3;
		break;
	case 3:
		/* battery */
		a += 4;
		sequence = ADCCON3_EREF_1_25 | ADCCON3_EDIV_512 | ADCCON3_ECH_TEMP;
		break;
	case ADCCON3_ECH_TEMP:
		a += 2;
		sequence = 0;
		break;
	}
#define GOT_ADC
#endif
	a[0] = ADCL;
	a[1] = ADCH;
	if (sequence) {
		/* Start next conversion */
		ADCCON3 = sequence;
		return;
	}
#endif /* telemini || telenano */

#if defined(TELEFIRE_V_0_1) || defined(TELEFIRE_V_0_2)
	a = (uint8_t __xdata *) (&ao_data_ring[ao_data_head].adc.sense[0] + sequence - AO_ADC_FIRST_PIN);
	a[0] = ADCL;
	a[1] = ADCH;
	if (sequence < 5) {
		ADCCON3 = ADCCON3_EREF_VDD | ADCCON3_EDIV_512 | (sequence + 1);
		return;
	}
#define GOT_ADC
#endif /* TELEFIRE_V_0_1 */

#ifdef TELEBT_V_1_0
	a = (uint8_t __xdata *) (&ao_data_ring[ao_data_head].adc.batt);
	a[0] = ADCL;
	a[1] = ADCH;
#define GOT_ADC
#endif	

#ifdef FETCH_ADC
	FETCH_ADC();
#define GOT_ADC
#endif

#ifndef GOT_ADC
#error No known ADC configuration set
#endif

	/* record this conversion series */
	ao_data_ring[ao_data_head].tick = ao_time();
	ao_data_head = ao_data_ring_next(ao_data_head);
	ao_wakeup(DATA_TO_XDATA(&ao_data_head));
}

static void
ao_adc_dump(void) __reentrant
{
	static __xdata struct ao_data	packet;
	ao_data_get(&packet);
#ifndef AO_ADC_DUMP
	printf("tick: %5u accel: %5d pres: %5d temp: %5d batt: %5d drogue: %5d main: %5d\n",
	       packet.tick, packet.adc.accel, packet.adc.pres, packet.adc.temp,
	       packet.adc.v_batt, packet.adc.sense_d, packet.adc.sense_m);
#else
	AO_ADC_DUMP(&packet);
#endif
}

__code struct ao_cmds ao_adc_cmds[] = {
	{ ao_adc_dump,	"a\0Current ADC" },
	{ 0, NULL },
};

void
ao_adc_init(void)
{
#ifdef AO_ADC_PINS
	ADCCFG = AO_ADC_PINS;

#else

#if IGNITE_ON_P2
	/* TeleMetrum configuration */
	ADCCFG = ((1 << 0) |	/* acceleration */
		  (1 << 1) |	/* pressure */
#if HAS_EXTERNAL_TEMP
		  (1 << 2) |	/* v0.1 temperature */
#endif
		  (1 << 3) |	/* battery voltage */
		  (1 << 4) |	/* drogue sense */
		  (1 << 5));	/* main sense */
#endif

#if IGNITE_ON_P0
	/* TeleMini configuration */
	ADCCFG = ((1 << 0) |	/* pressure */
		  (1 << 1) |	/* drogue sense */
		  (1 << 2) |	/* main sense */
		  (1 << 3));	/* battery voltage */
#endif

#endif /* else AO_ADC_PINS */

	/* enable interrupts */
	ADCIF = 0;
	IEN0 |= IEN0_ADCIE;
	ao_cmd_register(&ao_adc_cmds[0]);
}

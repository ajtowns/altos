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


#ifdef AO_ADC_ADCCON3
#define AO_ADC_END_OF_LIST (0xFF)

static uint8_t *ao_adc_order_in;

static uint8_t __xdata *ao_adc_order_out;

static const uint8_t ao_adc_order_adccon3[] = {
	AO_ADC_ADCCON3, 
	AO_ADC_END_OF_LIST 
};

#else

#ifndef AO_ADC_FIRST_PIN
# if HAS_ACCEL_REF
#  define AO_ADC_FIRST_PIN	2
# else
#  define AO_ADC_FIRST_PIN	0
# endif
#endif

#endif

void
ao_adc_poll(void)
{
#ifdef AO_ADC_ADCCON3
	ao_adc_order_in = &ao_adc_order_adccon3[0];
	ao_adc_order_out = (uint8_t __xdata *) (&ao_data_ring[ao_data_head].adc);
	ADCCON3 = *ao_adc_order_in;
#else
	ADCCON3 = ADCCON3_EREF_VDD | ADCCON3_EDIV_512 | AO_ADC_FIRST_PIN;
#endif
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

#ifdef AO_ADC_ADCCON3

void
ao_adc_isr(void) __interrupt 1
{
	uint8_t next;

	*(ao_adc_order_out++) = ADCL;
	*(ao_adc_order_out++) = ADCH;

	next = *(++ao_adc_order_in);
	if (next != AO_ADC_END_OF_LIST) {
		ADCCON3 = next;
	} else {
		/* record this conversion series */
		ao_data_ring[ao_data_head].tick = ao_time();
		ao_data_head = ao_data_ring_next(ao_data_head);
		ao_wakeup(DATA_TO_XDATA(&ao_data_head));
	}
}

#else /* AO_ADC_ADCCON3 */
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
	}
#endif

#ifdef TELEFIRE_V_0_1
	a = (uint8_t __xdata *) (&ao_data_ring[ao_data_head].adc.sense[0] + sequence - AO_ADC_FIRST_PIN);
	a[0] = ADCL;
	a[1] = ADCH;
	if (sequence < 5)
		ADCCON3 = ADCCON3_EREF_VDD | ADCCON3_EDIV_512 | (sequence + 1);
#define GOT_ADC
#endif /* TELEFIRE_V_0_1 */

#ifndef GOT_ADC
#error No known ADC configuration set
#endif

	else {
		/* record this conversion series */
		ao_data_ring[ao_data_head].tick = ao_time();
		ao_data_head = ao_data_ring_next(ao_data_head);
		ao_wakeup(DATA_TO_XDATA(&ao_data_head));
	}
}
#endif /* AO_ADC_ADCCON3 */

static void
ao_adc_dump(void) __reentrant
{
	static __xdata struct ao_data	packet;
	ao_data_get(&packet);
	AO_ADC_DUMP(&packet);
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
#else
#error "Need to set AO_ADC_PINS"
#endif

#endif /* else AO_ADC_PINS */

	/* enable interrupts */
	ADCIF = 0;
	IEN0 |= IEN0_ADCIE;
	ao_cmd_register(&ao_adc_cmds[0]);
}

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

#ifndef AO_ADC_FIRST_PIN
# if HAS_ACCEL_REF
#  define AO_ADC_FIRST_PIN	2
# else
#  define AO_ADC_FIRST_PIN	0
# endif
#endif

#ifdef TELEMINI_V_1_0
static const uint8_t ao_adc_pin_dest[6] = { 2, 8, 10, 6 };
#define AO_ADC_TEMP_OFFSET 4
#endif

#ifdef TELENANO_V_0_1
static const uint8_t ao_adc_pin_dest[6] = { ~0, 2, ~0, 6 };
#define AO_ADC_TEMP_OFFSET 4
#endif

#if TELEMETRUM_V_0_1 || TELEMETRUM_V_0_2 || TELEMETRUM_V_1_0 || TELEMETRUM_V_1_1 || TELEMETRUM_V_1_2 || TELELAUNCH_V_0_1 || TELEBALLOON_V_1_1

# if HAS_ACCEL_REF
#  define AO_ADC_ACCEL_PIN 2
#  define AO_ADC_ACCEL_OFFSET 12
# endif

# if HAS_EXTERNAL_TEMP == 0
static const uint8_t ao_adc_pin_dest[6] = { 0, 2, ~0, 6, 8, 10 };
#  define AO_ADC_TEMP_OFFSET 4
# else
static const uint8_t ao_adc_pin_dest[6] = { 0, 2, 4, 6, 8, 10 };
# endif

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
#if TELEMETRUM_V_0_1 || TELEMETRUM_V_0_2 || TELEMETRUM_V_1_0 || TELEMETRUM_V_1_1 || TELEMETRUM_V_1_2 || TELELAUNCH_V_0_1 || TELEBALLOON_V_1_1 || TELEMINI_V_1_0 || TELENANO_V_0_1
#define GOT_ADC
	a = (uint8_t __xdata *) (&ao_data_ring[ao_data_head].adc.accel);

#ifdef AO_ADC_TEMP_OFFSET
	if (sequence == ADCCON3_ECH_TEMP) {
		a += AO_ADC_TEMP_OFFSET;
		sequence = 0;
		goto store_adc;
	}
#endif

#if HAS_ACCEL_REF
	if (sequence == AO_ADC_ACCEL_PIN) {
		a += AO_ADC_ACCEL_OFFSET;
		sequence = 0;
		goto bump_sequence;
	}
#endif

	a += ao_adc_pin_dest[sequence];
	while (++sequence < sizeof(ao_adc_pin_dest)) {
bump_sequence:
		if (ao_adc_pin_dest[sequence] != (uint8_t) ~0) {
			sequence |= ADCCON3_EREF_VDD | ADCCON3_EDIV_512;
			goto store_adc;
		}
	}
#ifdef AO_ADC_TEMP_OFFSET_
	sequence = ADCCON3_EREF_1_25 | ADCCON3_EDIV_512 | ADCCON3_ECH_TEMP;
#else
	sequence = 0;
#endif

store_adc:
	a[0] = ADCL;
	a[1] = ADCH;
	if (sequence) {
		/* Start next conversion */
		ADCCON3 = sequence;
	}
#endif /* telemini || telenano */

#ifdef TELEFIRE_V_0_1
	a = (uint8_t __xdata *) (&ao_data_ring[ao_data_head].adc.sense[0] + sequence - AO_ADC_FIRST_PIN);
	a[0] = ADCL;
	a[1] = ADCH;
	if (sequence < 5)
		ADCCON3 = ADCCON3_EREF_VDD | ADCCON3_EDIV_512 | (sequence + 1);
#define GOT_ADC
#endif /* TELEFIRE_V_0_1 */

	else {
		/* record this conversion series */
		ao_data_ring[ao_data_head].tick = ao_time();
		ao_data_head = ao_data_ring_next(ao_data_head);
		ao_wakeup(DATA_TO_XDATA(&ao_data_head));
	}
}

#ifndef GOT_ADC
#error No known ADC configuration set
#endif

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

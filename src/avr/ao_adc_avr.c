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

volatile __xdata struct ao_adc	ao_adc_ring[AO_ADC_RING];
volatile __data uint8_t		ao_adc_head;

const uint8_t	adc_channels[AO_LOG_TELESCIENCE_NUM_ADC] = {
	0x00,
	0x01,
	0x04,
	0x05,
	0x06,
	0x07,
	0x20,
	0x21,
	0x22,
	0x23,
	0x24,
	0x25,
};

static uint8_t	ao_adc_channel;

#define ADC_CHANNEL_LOW(c)	(((c) & 0x1f) << MUX0)
#define ADC_CHANNEL_HIGH(c)	((((c) & 0x20) >> 5) << MUX5)

#define ADCSRA_INIT	((1 << ADEN) |		/* Enable ADC */ 		\
			 (0 << ADATE) |		/* No auto ADC trigger */ 	\
			 (1 << ADIF) |		/* Clear interrupt */		\
			 (0 << ADIE) |		/* Enable interrupt */		\
			 (6 << ADPS0))		/* Prescale clock by 64 */

#define ADCSRB_INIT	((0 << ADHSM) |		/* No high-speed mode */ \
			 (0 << ACME) |		/* Some comparitor thing */ \
			 (2 << ADTS0))		/* Free running mode (don't care) */

static void
ao_adc_start(void)
{
	uint8_t	channel = adc_channels[ao_adc_channel];
	ADMUX = ((0 << REFS1) |				/* AVcc reference */
		 (1 << REFS0) |				/* AVcc reference */
		 (1 << ADLAR) |				/* Left-shift results */
		 (ADC_CHANNEL_LOW(channel)));		/* Select channel */

	ADCSRB = (ADCSRB_INIT |
		  ADC_CHANNEL_HIGH(channel));		/* High channel bit */

	ADCSRA = (ADCSRA_INIT |
		  (1 << ADSC) |
		  (1 << ADIE));				/* Start conversion */
}

ISR(ADC_vect)
{
	uint16_t	value;

	/* Must read ADCL first or the value there will be lost */
	value = ADCL;
	value |= (ADCH << 8);
	ao_adc_ring[ao_adc_head].adc[ao_adc_channel] = value;
	if (++ao_adc_channel < AO_TELESCIENCE_NUM_ADC)
		ao_adc_start();
	else {
		ADCSRA = ADCSRA_INIT;
		ao_adc_ring[ao_adc_head].tick = ao_time();
		ao_adc_head = ao_adc_ring_next(ao_adc_head);
		ao_wakeup((void *) &ao_adc_head);
		ao_cpu_sleep_disable = 0;
	}
}

void
ao_adc_poll(void)
{
	ao_cpu_sleep_disable = 1;
	ao_adc_channel = 0;
	ao_adc_start();
}

void
ao_adc_get(__xdata struct ao_adc *packet)
{
	uint8_t	i = ao_adc_ring_prev(ao_adc_head);
	memcpy(packet, (void *) &ao_adc_ring[i], sizeof (struct ao_adc));
}

static void
ao_adc_dump(void) __reentrant
{
	static __xdata struct ao_adc	packet;
	uint8_t i;
	ao_adc_get(&packet);
	printf("tick: %5u",  packet.tick);
	for (i = 0; i < AO_TELESCIENCE_NUM_ADC; i++)
		printf (" %2d: %5u", i, packet.adc[i]);
	printf ("\n");
}

__code struct ao_cmds ao_adc_cmds[] = {
	{ ao_adc_dump,	"a\0Display current ADC values" },
	{ 0, NULL },
};

void
ao_adc_init(void)
{
	DIDR0 = 0xf3;
	DIDR2 = 0x3f;
	ADCSRB = ADCSRB_INIT;
	ADCSRA = ADCSRA_INIT;
	ao_cmd_register(&ao_adc_cmds[0]);
}
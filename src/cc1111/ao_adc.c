/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
 * Copyright © 2012 Anthony Towns <aj@erisian.com.au>
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

#define COMPILE_ASSERT(x) extern int compile_assert[1-2*!(x)]

#ifndef AO_ADC_SETUP
#error No known ADC configuration set
#endif

#ifndef AO_ADC_PINS
#define ao_adc_pins(t,n,pin)	((pin) < 8 ? (1 << (pin)) : 0) |
#define AO_ADC_PINS		AO_ADC_SETUP(ao_adc_pins) 0
#endif

#define ao_adc_cfg_array(t,n,pin)  ((pin) < 8 ? AO_ADC_PIN((pin)) : (pin)),
#define AO_ADC_END_OF_LIST	   (0xFF)

static const uint8_t ao_adc_order_adccon3[] = {
	AO_ADC_SETUP(ao_adc_cfg_array)
	AO_ADC_END_OF_LIST 
};

static uint8_t		*ao_adc_order_in;
static uint8_t __xdata	*ao_adc_order_out;

void
ao_adc_poll(void)
{
	ao_adc_order_in = &ao_adc_order_adccon3[0];
	ao_adc_order_out = (uint8_t __xdata *) (&ao_data_ring[ao_data_head].adc);
	ADCCON3 = ao_adc_order_adccon3[0];
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

#define ao_adc_printf_fmt(t,name,p)     " " #name ": %5d"
#define ao_adc_printf_arg(t,name,p)     , packet.adc. name

static void
ao_adc_dump(void) __reentrant
{
	static __xdata struct ao_data	packet;
	ao_data_get(&packet);
#ifdef AO_ADC_DUMP
	AO_ADC_DUMP(&packet);
#else
	printf( "tick: %5u"  AO_ADC_SETUP(ao_adc_printf_fmt) "\n",
		packet.tick  AO_ADC_SETUP(ao_adc_printf_arg) );

#endif
}

__code struct ao_cmds ao_adc_cmds[] = {
	{ ao_adc_dump,	"a\0Current ADC" },
	{ 0, NULL },
};

void
ao_adc_init(void)
{
	ADCCFG = AO_ADC_PINS;

	/* enable interrupts */
	ADCIF = 0;
	IEN0 |= IEN0_ADCIE;
	ao_cmd_register(&ao_adc_cmds[0]);
}

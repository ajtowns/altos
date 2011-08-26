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

static volatile __data uint16_t ao_tick_count;

uint16_t ao_time(void)
{
	uint16_t	v;
	ao_arch_critical(
		v = ao_tick_count;
		);
	return v;
}

static __xdata uint8_t ao_forever;

void
ao_delay(uint16_t ticks)
{
	ao_alarm(ticks);
	ao_sleep(&ao_forever);
}

#define T1_CLOCK_DIVISOR	8	/* 24e6/8 = 3e6 */
#define T1_SAMPLE_TIME		30000	/* 3e6/30000 = 100 */

#if HAS_ADC
volatile __data uint8_t	ao_adc_interval = 1;
volatile __data uint8_t	ao_adc_count;
#endif

void
ao_debug_out(char c);

ISR(TIMER1_COMPA_vect)
{
	++ao_tick_count;
#if HAS_ADC
	if (++ao_adc_count == ao_adc_interval) {
		ao_adc_count = 0;
		ao_adc_poll();
	}
#endif
}

#if HAS_ADC
void
ao_timer_set_adc_interval(uint8_t interval) __critical
{
	ao_adc_interval = interval;
	ao_adc_count = 0;
}
#endif

void
ao_timer_init(void)
{
	TCCR1A = ((0 << WGM11) |	/* CTC mode, OCR1A */
		  (0 << WGM10));	/* CTC mode, OCR1A */
	TCCR1B = ((0 << ICNC1) |	/* no input capture noise canceler */
		  (0 << ICES1) |	/* input capture on falling edge (don't care) */
		  (0 << WGM13) |	/* CTC mode, OCR1A */
		  (1 << WGM12) |	/* CTC mode, OCR1A */
		  (3 << CS10));		/* clk/64 from prescaler */

#if TEENSY
	OCR1A = 2500;			/* 16MHz clock */
#else
	OCR1A = 1250;			/* 8MHz clock */
#endif

	TIMSK1 = (1 << OCIE1A);		/* Interrupt on compare match */
}

/*
 * AltOS always cranks the clock to the max frequency
 */
void
ao_clock_init(void)
{
	/* disable RC clock */
	CLKSEL0 &= ~(1 << RCE);

	/* Disable PLL */
	PLLCSR &= ~(1 << PLLE);

	/* Enable external clock */
	CLKSEL0 |= (1 << EXTE);

	/* wait for external clock to be ready */
	while ((CLKSTA & (1 << EXTON)) == 0)
		;

	/* select external clock */
	CLKSEL0 |= (1 << CLKS);

	/* Disable the clock prescaler */
	cli();
	CLKPR = (1 << CLKPCE);

	/* Always run the system clock at 8MHz */
#if AVR_CLOCK > 12000000UL
	CLKPR = 1;
#else
	CLKPR = 0;
#endif
	sei();

	/* Set up the PLL to use the crystal */

	/* Use primary system clock as PLL source */
	PLLFRQ = ((0 << PINMUX) |	/* Use primary clock */
		  (0 << PLLUSB) |	/* No divide by 2 for USB */
		  (0 << PLLTM0) |	/* Disable high speed timer */
		  (0x4 << PDIV0));	/* 48MHz PLL clock */

	/* Set the frequency of the crystal */
#if AVR_CLOCK > 12000000UL
	PLLCSR |= (1 << PINDIV);	/* For 16MHz crystal on Teensy board */
#else
	PLLCSR &= ~(1 << PINDIV);	/* For 8MHz crystal on TeleScience board */
#endif

	/* Enable the PLL */
	PLLCSR |= (1 << PLLE);
	while (!(PLLCSR & (1 << PLOCK)))
		;

	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_enable();
}

/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

#include <ao.h>

volatile AO_TICK_TYPE	ao_tick_count;
static volatile AO_TICK_TYPE	ao_wakeup_count;

ISR(TIMER1_COMPA_vect)
{
	++ao_tick_count;
	if ((int16_t) (ao_tick_count - ao_wakeup_count) >= 0)
		ao_wakeup((void *) &ao_tick_count);
}

uint16_t
ao_time(void)
{
	uint16_t	r;

	cli();
	r = ao_tick_count;
	sei();
	return r;
}

#if AVR_CLOCK == 8000000UL
#define AO_CLKPS	0	/* divide by 1 */
#define AO_CS		10	/* prescale by 512 */
#endif
#if AVR_CLOCK == 4000000UL
#define AO_CLKPS	1	/* divide by 2 */
#define AO_CS		9	/* prescale by 256 */
#endif
#if AVR_CLOCK == 2000000UL
#define AO_CLKPS	2	/* divide by 4 */
#define AO_CS		8	/* prescale by 128 */
#endif
#if AVR_CLOCK == 1000000UL
#define AO_CLKPS	3	/* divide by 8 */
#define AO_CS		7	/* prescale by 64 */
#endif
#if AVR_CLOCK == 500000UL
#define AO_CLKPS	4	/* divide by 16 */
#define AO_CS		6	/* prescale by 32 */
#endif
#if AVR_CLOCK == 250000UL
#define AO_CLKPS	5	/* divide by 32 */
#define AO_CS		5	/* prescale by 16 */
#endif
#if AVR_CLOCK == 125000UL
#define AO_CLKPS	6	/* divide by 64 */
#define AO_CS		4	/* prescale by 32 */
#endif
#if AVR_CLOCK == 62500UL
#define AO_CLKPS	7	/* divide by 128 */
#define AO_CS		4	/* prescale by 32 */
#endif

void
ao_timer_init(void)
{
	cli();
	CLKPR = (1 << CLKPCE);
	CLKPR = (AO_CLKPS << CLKPS0);
	sei();

	/* Overall division ratio is 512 * 125,
	 * so our 8MHz base clock ends up as a 125Hz
	 * clock
	 */
	TCCR1 = ((1 << CTC1) |		/* Clear timer on match */
		 (0 << PWM1A) |		/* Not PWM mode */
		 (0 << COM1A0) |	/* Don't change output pins */
		 (0 << COM1A1) |	/*  ... */
		 (AO_CS << CS10));	/* Prescale */
	GTCCR = ((0 << PWM1B) |		/* Not PWM mode */
		 (0 << COM1B1) |	/* Don't change output pins */
		 (0 << COM1B0) |	/*  ... */
		 (0 << FOC1B) |		/* Don't force output compare */
		 (0 << FOC1A) |		/*  ... */
		 (0 << PSR1));		/* Don't bother to reset scaler */

	OCR1A = 0;
	OCR1B = 0;
	OCR1C = 124;			/* Divide by as many 5s as we can (5^3 = 125) */

	TIMSK = ((1 << OCIE1A) |	/* Enable TIMER1_COMPA interrupt */
		 (0 << OCIE1B) |	/* Disable TIMER1_COMPB interrupt */
		 (0 << TOIE1));		/* Disable TIMER1_OVF interrupt */
	DDRB |= 2;
}

#define PER_LOOP	8
#define US_LOOPS	((AVR_CLOCK / 1000000) / PER_LOOP)

void ao_delay_us(uint16_t us)
{
#if US_LOOPS > 1
	us *= US_LOOPS;
#endif
	for (;;) {
		ao_arch_nop();
		ao_arch_nop();
		ao_arch_nop();
		--us;
		/* A bit funky to keep the optimizer
		 * from short-circuiting the test */
		if (!((uint8_t) (us | (us >> 8))))
			break;
	}
}

void
ao_delay_until(uint16_t target)
{
	cli();
	ao_wakeup_count = target;
	while ((int16_t) (target - ao_tick_count) > 0)
		ao_sleep((void *) &ao_tick_count);
	sei();
}

void
ao_delay(uint16_t ticks)
{
	ao_delay_until(ao_time() + ticks);
}


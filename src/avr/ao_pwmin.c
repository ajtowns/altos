/*
 * Copyright © 2012 Robert D. Garbee <robert@gag.com>
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
#include "ao_pwmin.h"

/* 
 * This code implements a PWM input using ICP3.  
 *
 * The initial use is to measure wind speed in the ULA/Ball summer intern 
 * project payload developed at Challenger Middle School.  
 */

volatile __data uint16_t ao_tick3_count;

static void
ao_pwmin_display(void) __reentrant
{
	uint8_t lo = TCNT1L; 
	uint8_t hi = TCNT1H;
	uint16_t value = (hi <<8) | lo;

	uint8_t lo3 = TCNT3L; 
	uint8_t hi3 = TCNT3H;
	uint16_t value3 = (hi3 <<8) | lo3;

	/* now display the value we read */
	printf("timer 1: %5u %2x %2x\n", value, hi, lo);
	printf("timer 3: %5u %2x %2x\n", value3, hi3, lo3);

}
ISR(TIMER3_COMPA_vect)
{
        ++ao_tick3_count;
}

__code struct ao_cmds ao_pwmin_cmds[] = {
	{ ao_pwmin_display,	"p\0PWM input" },
	{ 0, NULL },
};

void
ao_pwmin_init(void)
{
	/* do hardware setup here */
	TCCR3A = ((0 << WGM31) |        /* normal mode, OCR3A */
                  (0 << WGM30));        /* normal mode, OCR3A */
        TCCR3B = ((0 << ICNC3) |        /* no input capture noise canceler */
                  (0 << ICES3) |        /* input capture on falling edge (don't care) */
                  (0 << WGM33) |        /* normal mode, OCR3A */
                  (0 << WGM32) |        /* normal mode, OCR3A */
                  (4 << CS30));         /* clk/256 from prescaler */

        OCR3A = 1250;                   /* 8MHz clock */

        TIMSK3 = (1 << OCIE3A);         /* Interrupt on compare match */

		/* set the spike filter bit in the TCCR3B register */

	ao_cmd_register(&ao_pwmin_cmds[0]);
}



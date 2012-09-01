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

volatile __data uint16_t ao_icp3_count;

static void
ao_pwmin_display(void) __reentrant
{
	/* display the most recent value */
	printf("icp 3: %5u\n", ao_icp3_count);

}
ISR(TIMER3_CAPT_vect)
{
	uint8_t lo = ICR3L; 
	uint8_t hi = ICR3H;
	ao_icp3_count = (hi <<8) | lo;
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
        TCCR3B = ((1 << ICNC3) |        /* input capture noise canceler on */
                  (0 << ICES3) |        /* input capture on falling edge (don't care) */
                  (0 << WGM33) |        /* normal mode, OCR3A */
                  (0 << WGM32) |        /* normal mode, OCR3A */
                  (4 << CS30));         /* clk/256 from prescaler */

    	

        TIMSK3 = (1 << ICIE3);         /* Interrupt on input compare */

		/* set the spike filter bit in the TCCR3B register */

	ao_cmd_register(&ao_pwmin_cmds[0]);
}



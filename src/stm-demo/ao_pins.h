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

#ifndef _AO_PINS_H_
#define _AO_PINS_H_

#define HAS_SERIAL_1		1
#define HAS_SERIAL_2		1
#define HAS_SERIAL_3		1
#define USE_SERIAL_STDIN	1
#define HAS_USB			0
#define HAS_BEEP		0
#define PACKET_HAS_SLAVE	0

#define LOW_LEVEL_DEBUG		1

#define LED_PORT_ENABLE		STM_RCC_AHBENR_GPIOBEN
#define LED_PORT		stm_gpiob
#define LED_PIN_GREEN		7
#define LED_PIN_BLUE		6
#define AO_LED_GREEN		(1 << LED_PIN_GREEN)
#define AO_LED_BLUE		(1 << LED_PIN_BLUE)

#define AO_LED_RED		AO_LED_BLUE	/* a patent lie */

#define LEDS_AVAILABLE		(AO_LED_BLUE | AO_LED_GREEN)

#endif /* _AO_PINS_H_ */

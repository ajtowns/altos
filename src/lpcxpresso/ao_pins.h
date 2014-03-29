/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

#define HAS_BEEP	0

/* Crystal on the board */
#define AO_LPC_CLKIN	12000000

/* Main clock frequency. 48MHz for USB so we don't use the USB PLL */
#define AO_LPC_CLKOUT	48000000

/* System clock frequency */
#define AO_LPC_SYSCLK	24000000

#define LED_PORT	0
#define LED_PIN_RED	7

#define AO_LED_RED	(1 << LED_PIN_RED)

#define LEDS_AVAILABLE	AO_LED_RED

#define HAS_USB		1

#define HAS_USB_CONNECT	1
#define HAS_USB_VBUS	1

#define PACKET_HAS_SLAVE	0

/* USART */

#define HAS_SERIAL		1
#define USE_SERIAL_0_STDIN	1
#define SERIAL_0_18_19		1
#define SERIAL_0_14_15		0
#define SERIAL_0_17_18		0
#define SERIAL_0_26_27		0

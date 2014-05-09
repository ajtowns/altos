/*
 * Copyright © 2014 Bdale Garbee <bdale@gag.com>
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

#define AO_STACK_SIZE	384

#define IS_FLASH_LOADER	0

/* Crystal on the board */
#define AO_LPC_CLKIN	12000000

/* Main clock frequency. 48MHz for USB so we don't use the USB PLL */
#define AO_LPC_CLKOUT	48000000

/* System clock frequency */
#define AO_LPC_SYSCLK	24000000

#define HAS_USB		1

#define HAS_USB_CONNECT	0
#define HAS_USB_VBUS	0
#define HAS_USB_PULLUP	1
#define AO_USB_PULLUP_PORT	0
#define AO_USB_PULLUP_PIN	6

/* USART */

#define HAS_SERIAL		1
#define USE_SERIAL_0_STDIN	0
#define SERIAL_0_18_19		1
#define SERIAL_0_14_15		0
#define SERIAL_0_17_18		0
#define SERIAL_0_26_27		0

/* SPI */

#define HAS_SPI_0		0
#define SPI_SCK0_P0_6		0
#define HAS_SPI_1		0
#define SPI_SCK1_P1_15		0
#define SPI_MISO1_P0_22		0
#define SPI_MOSI1_P0_21		0

/* LED */

#define LED_PORT		0
#define LED_PIN_RED		23
#define LED_PIN_GREEN		7
#define AO_LED_RED		(1 << LED_PIN_RED)
#define AO_LED_GREEN		(1 << LED_PIN_GREEN)

#define LEDS_AVAILABLE		(AO_LED_RED|AO_LED_GREEN)

/* RELAY */

#define RELAY_PORT		0
#define RELAY_PIN		3
#define RELAY_BIT		(1 << RELAY_PIN)

/* Kludge the SPI driver to not configure any
 * pin for SCK or MOSI
 */
#define HAS_SCK1		0
#define HAS_MOSI1		0

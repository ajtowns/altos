/*
 * Copyright © 2010 Keith Packard <keithp@keithp.com>
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

#define HAS_RADIO		1

#define HAS_FLIGHT		0
#define HAS_USB			0
#define HAS_BEEP		0
#define HAS_GPS			0
#define HAS_SERIAL_0		0
#define HAS_SERIAL_0_ALT_1	0
#define HAS_SERIAL_0_HW_FLOW	0
#define USE_SERIAL_0_STDIN	0
#define HAS_SERIAL_1		1
#define HAS_SERIAL_1_ALT_1	1
#define HAS_SERIAL_1_HW_FLOW	0
#define USE_SERIAL_1_STDIN	1
#define DELAY_SERIAL_1_STDIN	0
#define HAS_ADC			0
#define HAS_DBG			0
#define HAS_EEPROM		0
#define HAS_LOG			0
#define USE_INTERNAL_FLASH	0
#define DBG_ON_P1 		0
#define PACKET_HAS_MASTER	0
#define PACKET_HAS_SLAVE	0
#define AO_LED_TX		1
#define AO_LED_RX		2
#define AO_LED_RED		AO_LED_TX
#define LEDS_AVAILABLE		(AO_LED_TX|AO_LED_RX)
#define HAS_EXTERNAL_TEMP	0
#define HAS_ACCEL_REF		0
#define SPI_CS_ON_P1		1
#define HAS_AES			1

#define SPI_CS_PORT		P1
#define SPI_CS_SEL		P1SEL
#define SPI_CS_DIR		P1DIR
#define AO_SPI_SLAVE		1
#define HAS_SPI_0		1
#define SPI_0_ALT_2		1
#define HAS_SPI_1		0

#define AO_RADIO_SLAVE_INT_PORT	P1
#define AO_RADIO_SLAVE_INT_BIT	6
#define AO_RADIO_SLAVE_INT_PIN	P1_6

#endif /* _AO_PINS_H_ */

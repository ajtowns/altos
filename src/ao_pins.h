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

#if defined(TELEMETRUM_V_1_0)
	#define HAS_SERIAL_1		1
	#define HAS_ADC			1
	#define HAS_EEPROM		1
	#define HAS_DBG			1
	#define DBG_ON_P1 		1
	#define DBG_ON_P0 		0
	#define PACKET_HAS_MASTER	0
	#define PACKET_HAS_SLAVE	1

	#define AO_LED_RED		1
	#define LEDS_AVAILABLE		(AO_LED_RED)
	#define HAS_EXTERNAL_TEMP	0
	#define HAS_ACCEL_REF		0
#endif

#if defined(TELEMETRUM_V_1_1)
	#define HAS_SERIAL_1		1
	#define HAS_ADC			1
	#define HAS_EEPROM		1
	#define HAS_DBG			1
	#define DBG_ON_P1 		1
	#define DBG_ON_P0 		0
	#define PACKET_HAS_MASTER	0
	#define PACKET_HAS_SLAVE	1

	#define AO_LED_RED		1
	#define LEDS_AVAILABLE		(AO_LED_RED)
	#define HAS_EXTERNAL_TEMP	0
	#define HAS_ACCEL_REF		1
	#define SPI_CS_ON_P1		1
	#define SPI_CS_ON_P0		0
	#define M25_CS_MASK		0x02	/* CS0 is P1_1 */
	#define M25_MAX_CHIPS		1
#endif

#if defined(TELEDONGLE_V_0_2)
	#define HAS_SERIAL_1		0
	#define HAS_ADC			0
	#define HAS_DBG			1
	#define HAS_EEPROM		0
	#define DBG_ON_P1		1
	#define DBG_ON_P0 		0
	#define PACKET_HAS_MASTER	1
	#define PACKET_HAS_SLAVE	0
	#define AO_LED_RED		1
	#define AO_LED_GREEN		2
	#define LEDS_AVAILABLE		(AO_LED_RED|AO_LED_GREEN)
	#define SPI_CS_ON_P1		1
	#define SPI_CS_ON_P0		0
#endif

#if defined(TELEMETRUM_V_0_1)
	#define HAS_SERIAL_1		1
	#define HAS_ADC			1
	#define HAS_DBG			0
	#define HAS_EEPROM		1
	#define DBG_ON_P1 		0
	#define DBG_ON_P0 		1
	#define PACKET_HAS_MASTER	0
	#define PACKET_HAS_SLAVE	1
	#define AO_LED_RED		2
	#define AO_LED_GREEN		1
	#define LEDS_AVAILABLE		(AO_LED_RED|AO_LED_GREEN)
	#define HAS_EXTERNAL_TEMP	1
	#define HAS_ACCEL_REF		0
	#define SPI_CS_ON_P1		1
	#define SPI_CS_ON_P0		0
#endif

#if defined(TELEDONGLE_V_0_1)
	#define HAS_SERIAL_1		0
	#define HAS_ADC			0
	#define HAS_DBG			0
	#define HAS_EEPROM		0
	#define DBG_ON_P1		0
	#define DBG_ON_P0 		1
	#define PACKET_HAS_MASTER	1
	#define PACKET_HAS_SLAVE	0
	#define AO_LED_RED		2
	#define AO_LED_GREEN		1
	#define LEDS_AVAILABLE		(AO_LED_RED|AO_LED_GREEN)
	#define SPI_CS_ON_P1		0
	#define SPI_CS_ON_P0		1
#endif

#if defined(TIDONGLE)
	#define HAS_SERIAL_1		0
	#define HAS_ADC			0
	#define HAS_DBG			1
	#define HAS_EEPROM		0
	#define DBG_ON_P1		0
	#define DBG_ON_P0		1
	#define PACKET_HAS_MASTER	1
	#define PACKET_HAS_SLAVE	0
	#define AO_LED_RED		2
	#define LEDS_AVAILABLE		(AO_LED_RED)
	#define SPI_CS_ON_P1		0
	#define SPI_CS_ON_P0		1
#endif

#if DBG_ON_P1

	#define DBG_CLOCK	(1 << 4)	/* mi0 */
	#define DBG_DATA	(1 << 5)	/* mo0 */
	#define DBG_RESET_N	(1 << 3)	/* c0 */

	#define DBG_CLOCK_PIN	(P1_4)
	#define DBG_DATA_PIN	(P1_5)
	#define DBG_RESET_N_PIN	(P1_3)

	#define DBG_PORT_NUM	1
	#define DBG_PORT	P1
	#define DBG_PORT_SEL	P1SEL
	#define DBG_PORT_INP	P1INP
	#define DBG_PORT_DIR	P1DIR

#endif /* DBG_ON_P1 */

#if DBG_ON_P0

	#define DBG_CLOCK	(1 << 3)
	#define DBG_DATA	(1 << 4)
	#define DBG_RESET_N	(1 << 5)

	#define DBG_CLOCK_PIN	(P0_3)
	#define DBG_DATA_PIN	(P0_4)
	#define DBG_RESET_N_PIN	(P0_5)

	#define DBG_PORT_NUM	0
	#define DBG_PORT	P0
	#define DBG_PORT_SEL	P0SEL
	#define DBG_PORT_INP	P0INP
	#define DBG_PORT_DIR	P0DIR

#endif /* DBG_ON_P0 */

#if SPI_CS_ON_P1
	#define SPI_CS_PORT	P1
	#define SPI_CS_SEL	P1SEL
	#define SPI_CS_DIR	P1DIR
#endif

#if SPI_CS_ON_P0
	#define SPI_CS_PORT	P0
	#define SPI_CS_SEL	P0SEL
	#define SPI_CS_DIR	P0DIR
#endif

#ifndef HAS_SERIAL_1
#error Please define HAS_SERIAL_1
#endif

#ifndef HAS_ADC
#error Please define HAS_ADC
#endif

#ifndef HAS_EEPROM
#error Please define HAS_EEPROM
#endif

#ifndef HAS_DBG
#error Please define HAS_DBG
#endif

#ifndef PACKET_HAS_MASTER
#error Please define PACKET_HAS_MASTER
#endif

#ifndef PACKET_HAS_SLAVE
#error Please define PACKET_HAS_SLAVE
#endif

#endif /* _AO_PINS_H_ */

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
	#define HAS_FLIGHT		1
	#define HAS_USB			1
	#define HAS_BEEP		1
	#define HAS_GPS			1
	#define HAS_SERIAL_1		1
	#define HAS_ADC			1
	#define USE_SERIAL_STDIN	0
	#define HAS_EEPROM		1
	#define USE_INTERNAL_FLASH	0
	#define HAS_DBG			1
	#define DBG_ON_P1 		1
	#define DBG_ON_P0 		0
	#define IGNITE_ON_P2		1
	#define IGNITE_ON_P0		0
	#define PACKET_HAS_MASTER	0
	#define PACKET_HAS_SLAVE	1

	#define HAS_COMPANION		1
	#define COMPANION_CS_ON_P1	1
	#define COMPANION_CS_MASK	0x4	/* CS1 is P1_2 */
	#define COMPANION_CS		P1_2

	#define AO_LED_RED		1
	#define LEDS_AVAILABLE		(AO_LED_RED)
	#define HAS_EXTERNAL_TEMP	0
	#define HAS_ACCEL_REF		0
	#define HAS_ACCEL		1
	#define HAS_IGNITE		1
	#define HAS_MONITOR		0
#endif

#if defined(TELEMETRUM_V_1_1)
	#define HAS_FLIGHT		1
	#define HAS_USB			1
	#define HAS_BEEP		1
	#define HAS_GPS			1
	#define HAS_SERIAL_1		1
	#define USE_SERIAL_STDIN	0
	#define HAS_ADC			1
	#define HAS_EEPROM		1
	#define USE_INTERNAL_FLASH	0
	#define HAS_DBG			1
	#define DBG_ON_P1 		1
	#define DBG_ON_P0 		0
	#define IGNITE_ON_P2		1
	#define IGNITE_ON_P0		0
	#define PACKET_HAS_MASTER	0
	#define PACKET_HAS_SLAVE	1

	#define HAS_COMPANION		1
	#define COMPANION_CS_ON_P1	1
	#define COMPANION_CS_MASK	0x4	/* CS1 is P1_2 */
	#define COMPANION_CS		P1_2

	#define AO_LED_RED		1
	#define LEDS_AVAILABLE		(AO_LED_RED)
	#define HAS_EXTERNAL_TEMP	0
	#define HAS_ACCEL_REF		1
	#define SPI_CS_ON_P1		1
	#define SPI_CS_ON_P0		0
	#define M25_CS_MASK		0x02	/* CS0 is P1_1 */
	#define M25_MAX_CHIPS		1
	#define HAS_ACCEL		1
	#define HAS_IGNITE		1
	#define HAS_MONITOR		0
#endif

#if defined(TELEDONGLE_V_0_2)
	#define HAS_FLIGHT		0
	#define HAS_USB			1
	#define HAS_BEEP		0
	#define HAS_SERIAL_1		0
	#define USE_SERIAL_STDIN	0
	#define HAS_ADC			0
	#define HAS_DBG			1
	#define HAS_EEPROM		0
	#define DBG_ON_P1		1
	#define DBG_ON_P0 		0
	#define IGNITE_ON_P2		0
	#define IGNITE_ON_P0		0
	#define PACKET_HAS_MASTER	1
	#define PACKET_HAS_SLAVE	0
	#define AO_LED_RED		1
	#define AO_LED_GREEN		2
	#define LEDS_AVAILABLE		(AO_LED_RED|AO_LED_GREEN)
	#define SPI_CS_ON_P1		1
	#define SPI_CS_ON_P0		0
	#define HAS_IGNITE		0
	#define HAS_MONITOR		1
#endif

#if defined(TELEMINI_V_1_0)
	#define HAS_FLIGHT		1
	#define HAS_USB			0
	#define HAS_BEEP		0
	#define HAS_GPS			0
	#define HAS_SERIAL_1		0
	#define USE_SERIAL_STDIN	0
	#define HAS_ADC			1
	#define HAS_EEPROM		1
	#define USE_INTERNAL_FLASH	1
	#define HAS_DBG			0
	#define IGNITE_ON_P2		0
	#define IGNITE_ON_P0		1
	#define PACKET_HAS_MASTER	0
	#define PACKET_HAS_SLAVE	1
	#define USE_FAST_ASCENT_LOG	1

	#define AO_LED_GREEN		1
	#define AO_LED_RED		2
	#define LEDS_AVAILABLE		(AO_LED_RED|AO_LED_GREEN)
	#define HAS_EXTERNAL_TEMP	0
	#define HAS_ACCEL		0
	#define HAS_IGNITE		1
	#define HAS_MONITOR		0
#endif

#if defined(TELENANO_V_0_1)
	#define HAS_FLIGHT		1
	#define HAS_USB			0
	#define HAS_BEEP		0
	#define HAS_GPS			0
	#define HAS_SERIAL_1		0
	#define USE_SERIAL_STDIN	0
	#define HAS_ADC			1
	#define HAS_EEPROM		1
	#define USE_INTERNAL_FLASH	1
	#define HAS_DBG			0
	#define IGNITE_ON_P2		0
	#define IGNITE_ON_P0		1
	#define PACKET_HAS_MASTER	0
	#define PACKET_HAS_SLAVE	1

	#define AO_LED_GREEN		1
	#define AO_LED_RED		2
	#define LEDS_AVAILABLE		(AO_LED_RED|AO_LED_GREEN)
	#define HAS_EXTERNAL_TEMP	0
	#define HAS_ACCEL		0
	#define HAS_IGNITE		0
	#define HAS_MONITOR		0
#endif

#if defined(TELEMETRUM_V_0_1)
	#define HAS_FLIGHT		1
	#define HAS_USB			1
	#define HAS_BEEP		1
	#define HAS_GPS			1
	#define HAS_SERIAL_1		1
	#define USE_SERIAL_STDIN	0
	#define HAS_ADC			1
	#define HAS_DBG			0
	#define HAS_EEPROM		1
	#define USE_INTERNAL_FLASH	0
	#define DBG_ON_P1 		0
	#define DBG_ON_P0 		1
	#define IGNITE_ON_P2		1
	#define IGNITE_ON_P0		0
	#define PACKET_HAS_MASTER	0
	#define PACKET_HAS_SLAVE	1
	#define AO_LED_RED		2
	#define AO_LED_GREEN		1
	#define LEDS_AVAILABLE		(AO_LED_RED|AO_LED_GREEN)
	#define HAS_EXTERNAL_TEMP	1
	#define HAS_ACCEL_REF		0
	#define SPI_CS_ON_P1		1
	#define SPI_CS_ON_P0		0
	#define HAS_ACCEL		1
	#define HAS_IGNITE		1
	#define HAS_MONITOR		0
#endif

#if defined(TELEDONGLE_V_0_1)
	#define HAS_FLIGHT		0
	#define HAS_USB			1
	#define HAS_BEEP		0
	#define HAS_SERIAL_1		0
	#define USE_SERIAL_STDIN	0
	#define HAS_ADC			0
	#define HAS_DBG			0
	#define HAS_EEPROM		0
	#define DBG_ON_P1		0
	#define DBG_ON_P0 		1
	#define IGNITE_ON_P2		0
	#define IGNITE_ON_P0		0
	#define PACKET_HAS_MASTER	1
	#define PACKET_HAS_SLAVE	0
	#define AO_LED_RED		2
	#define AO_LED_GREEN		1
	#define LEDS_AVAILABLE		(AO_LED_RED|AO_LED_GREEN)
	#define SPI_CS_ON_P1		0
	#define SPI_CS_ON_P0		1
	#define HAS_IGNITE		0
	#define HAS_MONITOR		1
#endif

#if defined(TIDONGLE)
	#define HAS_FLIGHT		0
	#define HAS_USB			1
	#define HAS_BEEP		0
	#define HAS_SERIAL_1		0
	#define USE_SERIAL_STDIN	0
	#define HAS_ADC			0
	#define HAS_DBG			1
	#define HAS_EEPROM		0
	#define DBG_ON_P1		0
	#define DBG_ON_P0		1
	#define IGNITE_ON_P2		0
	#define IGNITE_ON_P0		0
	#define PACKET_HAS_MASTER	1
	#define PACKET_HAS_SLAVE	0
	#define AO_LED_RED		2
	#define LEDS_AVAILABLE		(AO_LED_RED)
	#define SPI_CS_ON_P1		0
	#define SPI_CS_ON_P0		1
	#define HAS_IGNITE		0
	#define HAS_MONITOR		1
#endif

#if defined(TELEBT_V_0_0)
	#define HAS_FLIGHT		0
	#define HAS_USB			1
	#define HAS_BEEP		0
	#define HAS_SERIAL_1		1
	#define USE_SERIAL_STDIN	1
	#define HAS_ADC			0
	#define HAS_DBG			1
	#define HAS_EEPROM		0
	#define HAS_BTM			1
	#define DBG_ON_P1 		0
	#define DBG_ON_P0 		1
	#define IGNITE_ON_P2		0
	#define IGNITE_ON_P0		0
	#define PACKET_HAS_MASTER	1
	#define PACKET_HAS_SLAVE	0
	#define AO_LED_RED		2
	#define AO_LED_GREEN		1
	#define LEDS_AVAILABLE		(AO_LED_RED|AO_LED_GREEN)
	#define SPI_CS_ON_P1		1
	#define SPI_CS_ON_P0		0
	#define HAS_IGNITE		0
	#define BT_LINK_ON_P2		1
	#define BT_LINK_ON_P1		0
	#define BT_LINK_PIN_INDEX	7
	#define BT_LINK_PIN		P2_1
	#define HAS_MONITOR		1
#endif

#if defined(TELEBT_V_0_1)
	#define HAS_FLIGHT		0
	#define HAS_USB			1
	#define HAS_BEEP		1
	#define HAS_SERIAL_1		1
	#define HAS_SERIAL_1_ALT_1	1
	#define HAS_SERIAL_1_ALT_2	0
	#define HAS_SERIAL_1_HW_FLOW	1
	#define USE_SERIAL_STDIN	1
	#define HAS_ADC			0
	#define HAS_DBG			1
	#define HAS_EEPROM		1
	#define USE_INTERNAL_FLASH	0
	#define HAS_BTM			1
	#define DBG_ON_P1 		1
	#define DBG_ON_P0 		0
	#define IGNITE_ON_P2		0
	#define IGNITE_ON_P0		0
	#define PACKET_HAS_MASTER	1
	#define PACKET_HAS_SLAVE	0
	#define AO_LED_RED		1
	#define AO_LED_GREEN		2
	#define LEDS_AVAILABLE		(AO_LED_RED|AO_LED_GREEN)
	#define SPI_CS_ON_P1		1
	#define SPI_CS_ON_P0		0
	#define M25_CS_MASK		0x04	/* CS0 is P1_2 */
	#define M25_MAX_CHIPS		1
	#define HAS_ACCEL		0
	#define HAS_IGNITE		0
	#define BT_LINK_ON_P2		0
	#define BT_LINK_ON_P1		1
	#define BT_LINK_PIN_INDEX	7
	#define BT_LINK_PIN		P1_7
	#define HAS_MONITOR		1
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

#if COMPANION_CS_ON_P1
	#define COMPANION_CS_PORT	P1
	#define COMPANION_CS_SEL	P1SEL
	#define COMPANION_CS_DIR	P1DIR
#endif

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

#ifndef IGNITE_ON_P2
#error Please define IGNITE_ON_P2
#endif

#ifndef IGNITE_ON_P0
#error Please define IGNITE_ON_P0
#endif

#ifndef HAS_SERIAL_1
#error Please define HAS_SERIAL_1
#endif

#ifndef USE_SERIAL_STDIN
#error Please define USE_SERIAL_STDIN
#endif

#ifndef HAS_ADC
#error Please define HAS_ADC
#endif

#ifndef HAS_EEPROM
#error Please define HAS_EEPROM
#endif

#if HAS_EEPROM
#ifndef USE_INTERNAL_FLASH
#error Please define USE_INTERNAL_FLASH
#endif
#endif

#ifndef HAS_DBG
#error Please define HAS_DBG
#endif

#ifndef HAS_IGNITE
#error Please define HAS_IGNITE
#endif

#ifndef PACKET_HAS_MASTER
#error Please define PACKET_HAS_MASTER
#endif

#ifndef PACKET_HAS_SLAVE
#error Please define PACKET_HAS_SLAVE
#endif

#ifndef HAS_MONITOR
#error Please define HAS_MONITOR
#endif
#endif /* _AO_PINS_H_ */

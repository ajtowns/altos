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

#if defined(TELESHIELD_V_0_1)
	#define HAS_FLIGHT		0
	#define HAS_USB			1
	#define HAS_BEEP		0
	#define HAS_SERIAL_1		1
	#define HAS_SERIAL_1_ALT_1	1
	#define HAS_SERIAL_1_ALT_2	0
	#define HAS_SERIAL_1_HW_FLOW	1
	#define USE_SERIAL_1_STDIN	1
	#define HAS_SERIAL_0		1
	#define HAS_SERIAL_0_ALT_1	0
	#define HAS_SERIAL_0_ALT_2	1
	#define HAS_SERIAL_0_HW_FLOW	0
	#define HAS_ADC			0
	#define HAS_DBG			1
	#define HAS_EEPROM		1
	#define HAS_LOG			0
	#define USE_INTERNAL_FLASH	1
	#define HAS_BTM			1
	#define DBG_ON_P1 		1
	#define DBG_ON_P0 		0
	#define IGNITE_ON_P2		0
	#define IGNITE_ON_P0		0
	#define PACKET_HAS_MASTER	0
	#define PACKET_HAS_SLAVE	1
	#define AO_LED_RED		1
	#define AO_LED_GREEN		2
	#define AO_MONITOR_LED		AO_LED_RED
	#define AO_BT_LED		AO_LED_GREEN
	#define LEDS_AVAILABLE		(AO_LED_RED|AO_LED_GREEN)
	#define SPI_CS_ON_P1		1
	#define SPI_CS_ON_P0		0
	#define HAS_ACCEL		0
	#define HAS_IGNITE		0
	#define HAS_IGNITE_REPORT	0
	#define BT_LINK_ON_P2		0
	#define BT_LINK_ON_P1		1
	#define BT_LINK_PIN_INDEX	7
	#define BT_LINK_PIN		P1_7
	#define HAS_MONITOR		1
	#define LEGACY_MONITOR		0
	#define HAS_RSSI		0
	#define HAS_AES			0
	#define HAS_RADIO		1
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

#ifndef HAS_ADC
#error Please define HAS_ADC
#endif

#ifndef HAS_EEPROM
#error Please define HAS_EEPROM
#endif

#ifndef HAS_LOG
#error Please define HAS_LOG
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

#if HAS_IGNITE
#define HAS_IGNITE_REPORT 1
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

#if HAS_MONITOR
#ifndef HAS_RSSI
#error Please define HAS_RSSI
#endif
#endif

#ifndef HAS_ADC
#error Please define HAS_ADC
#endif

#if HAS_ADC

#if HAS_ACCEL
#ifndef HAS_ACCEL_REF
#error Please define HAS_ACCEL_REF
#endif
#else
#define HAS_ACCEL_REF 0
#endif

#endif /* HAS_ADC */

#if IGNITE_ON_P2
#define AO_IGNITER_DROGUE	P2_3
#define AO_IGNITER_MAIN		P2_4
#define AO_IGNITER_DIR		P2DIR
#define AO_IGNITER_DROGUE_BIT	(1 << 3)
#define AO_IGNITER_MAIN_BIT	(1 << 4)
#endif

#if IGNITE_ON_P0
#define AO_IGNITER_DROGUE	P0_5
#define AO_IGNITER_MAIN		P0_4
#define AO_IGNITER_DIR		P0DIR
#define AO_IGNITER_DROGUE_BIT	(1 << 5)
#define AO_IGNITER_MAIN_BIT	(1 << 4)
#endif

/* test these values with real igniters */
#define AO_IGNITER_OPEN		1000
#define AO_IGNITER_CLOSED	7000
#define AO_IGNITER_FIRE_TIME	AO_MS_TO_TICKS(50)
#define AO_IGNITER_CHARGE_TIME	AO_MS_TO_TICKS(2000)

void
ao_ardu_serial_init (void);


#endif /* _AO_PINS_H_ */

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

#define HAS_RADIO	1
#define HAS_FLIGHT		0
#define HAS_USB			1
#define HAS_BEEP		0
#define HAS_SERIAL_1		1
#define HAS_SERIAL_1_ALT_1	1
#define HAS_SERIAL_1_ALT_2	0
#define HAS_SERIAL_1_HW_FLOW	1
#define USE_SERIAL_1_STDIN	1
#define DELAY_SERIAL_1_STDIN	1
#define HAS_DBG			1
#define HAS_EEPROM		0
#define HAS_LOG			0
#define USE_INTERNAL_FLASH	0
#define HAS_BTM			1
#define DBG_ON_P1 		1
#define DBG_ON_P0 		0
#define PACKET_HAS_MASTER	1
#define PACKET_HAS_SLAVE	0
#define AO_LED_RED		1
#define AO_LED_BLUE		2
#define LEDS_AVAILABLE		(AO_LED_RED|AO_LED_BLUE)
#define AO_MONITOR_LED		AO_LED_RED
#define AO_BT_LED		AO_LED_BLUE
#define BT_LINK_ON_P2		0
#define BT_LINK_ON_P1		1
#define BT_LINK_PIN_INDEX	7
#define BT_LINK_PIN		P1_7
#define HAS_MONITOR		1
#define LEGACY_MONITOR		0

#define HAS_ADC			1
#define AO_PAD_ADC_BATT		0
#define AO_ADC_PINS		(1 << AO_PAD_ADC_BATT)

struct ao_adc {
	int16_t		batt;
};

#define AO_ADC_DUMP(p)							\
	printf ("tick: %5u batt %5d\n",					\
		(p)->tick,						\
		(p)->adc.batt)

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

#endif /* _AO_PINS_H_ */

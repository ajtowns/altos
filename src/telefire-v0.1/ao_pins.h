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
#define HAS_BEEP		1
#define HAS_GPS			0
#define HAS_SERIAL_1		0
#define HAS_ADC			1
#define HAS_DBG			0
#define HAS_EEPROM		1
#define HAS_LOG			0
#define HAS_PAD			1
#define USE_INTERNAL_FLASH	1
#define DBG_ON_P1 		0
#define IGNITE_ON_P2		0
#define IGNITE_ON_P1		1
#define IGNITE_ON_P0		0
#define PACKET_HAS_MASTER	0
#define PACKET_HAS_SLAVE	0

#define AO_LED_CONTINUITY(c)	(1 << ((c) + 2))
#define AO_LED_CONTINUITY_MASK	(0xc)
#define AO_LED_ARMED		0x10
#define AO_LED_RED		0x20
#define AO_LED_AMBER		0x40
#define AO_LED_GREEN		0x80

#define LEDS_AVAILABLE		(0xff)
#define HAS_EXTERNAL_TEMP	0
#define HAS_ACCEL_REF		0
#define SPI_CS_ON_P1		1
#define HAS_AES			1
#define DMA_SHARE_AES_RADIO	1

#define SPI_CS_PORT	P1
#define SPI_CS_SEL	P1SEL
#define SPI_CS_DIR	P1DIR

#define SPI_CONST	0x00

#define HAS_SPI_0		0
#define HAS_SPI_1		1
#define SPI_1_ALT_1		0
#define SPI_1_ALT_2		1

#define AO_74HC497_CS_PORT	P1
#define AO_74HC497_CS_PIN	4
#define AO_74HC497_CS		P1_4

#define AO_PCA9922_CS_PORT	P1
#define AO_PCA9922_CS_PIN	4
#define AO_PCA9922_CS		P1_4

#define AO_PAD_NUM		2
#define	AO_PAD_PORT		P1
#define AO_PAD_DIR		P1DIR

#define AO_PAD_PIN_0		2
#define AO_PAD_0		P1_2
#define AO_PAD_ADC_0		2

#define AO_PAD_PIN_1		3
#define AO_PAD_1		P1_3
#define AO_PAD_ADC_1		3

#define AO_PAD_ALL_PINS		((1 << AO_PAD_PIN_0) | (1 << AO_PAD_PIN_1))
#define AO_PAD_ALL_CHANNELS	((1 << 0) | (1 << 1))

#define AO_SIREN_PORT		P1
#define AO_SIREN_DIR		P1DIR
#define AO_SIREN_PIN		0
#define AO_SIREN		P1_0

#define AO_STROBE_PORT		P1
#define AO_STROBE_DIR		P1DIR
#define AO_STROBE_PIN		1
#define AO_STROBE		P1_1

/* test these values with real igniters */
#define AO_PAD_RELAY_CLOSED	3524
#define AO_PAD_NO_IGNITER	16904
#define AO_PAD_GOOD_IGNITER	22514

#define AO_PAD_ADC_PYRO		4
#define AO_PAD_ADC_BATT		5

#define AO_ADC_FIRST_PIN	2

struct ao_adc {
	int16_t		sense[AO_PAD_NUM];
	int16_t		pyro;
	int16_t		batt;
};

#define AO_ADC_DUMP(p)							\
	printf ("tick: %5u 0: %5d 1: %5d pyro: %5d batt %5d\n", 	\
		(p)->tick,						\
		(p)->adc.sense[0],					\
		(p)->adc.sense[1],					\
		(p)->adc.pyro,						\
		(p)->adc.batt)

#define AO_ADC_PINS	((1 << AO_PAD_ADC_0) | (1 << AO_PAD_ADC_1) | (1 << AO_PAD_ADC_PYRO) | (1 << AO_PAD_ADC_BATT))

#endif /* _AO_PINS_H_ */

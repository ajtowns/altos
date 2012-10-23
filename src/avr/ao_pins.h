/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

#ifdef AVR_DEMO
	#define AO_LED_RED		(1<<7)
	#define LEDS_AVAILABLE		(AO_LED_RED)
	#define USE_SERIAL_1_STDIN	1
	#define HAS_USB			1
	#define PACKET_HAS_SLAVE	0
	#define HAS_SERIAL_1		1
	#define TEENSY			1
	#define AVR_VCC_5V	       	1
	#define AVR_VCC_3V3		0
	#define AVR_CLOCK		16000000UL
	#define HAS_BEEP		0
#endif

#if defined(TELESCIENCE) || defined(TELESCIENCE_PWM)
	#define LEDS_AVAILABLE		0
	#define HAS_USB			1
	#define HAS_LOG			1
	#define TEENSY			0
	#define HAS_SERIAL_1		0
	#define HAS_ADC			1
	#define PACKET_HAS_SLAVE	0
	#define HAS_BEEP		0
	#define HAS_EEPROM		1
	#define HAS_STORAGE_DEBUG	0

	#define AVR_VCC_5V	       	0
	#define AVR_VCC_3V3		1
	#define AVR_CLOCK		8000000UL
#ifdef TELESCIENCE_PWM
	#define HAS_ICP3_COUNT		1
#else
	#define HAS_ICP3_COUNT		0
#endif

	#define SPI_CS_PORT		PORTE
	#define SPI_CS_DIR		DDRE
	#define M25_CS_MASK		(1 << PORTE6)
	#define M25_MAX_CHIPS		1

	#define SPI_SLAVE_CS_PORT	PORTB
	#define SPI_SLAVE_CS_PIN	PINB
	#define SPI_SLAVE_CS_PIN_NO	PINB0

	#define SPI_SLAVE_PIN_0_3	1
	#define SPI_SLAVE_PIN_2_5	0

	#define IS_COMPANION		1
#endif

#ifdef TELEPYRO
	#define AO_STACK_SIZE		104
	#define LEDS_AVAILABLE		0
	#define HAS_USB			1
	#define HAS_LOG			0
	#define TEENSY			0
	#define USE_SERIAL_1_STDIN	1
	#define HAS_SERIAL_1		1
	#define HAS_USB			1
	#define HAS_ADC			1
	#define PACKET_HAS_SLAVE	0
	#define HAS_BEEP		0
	#define HAS_EEPROM		1
	#define USE_INTERNAL_FLASH	1
	#define DISABLE_HELP		1
	#define HAS_STORAGE_DEBUG	0
	#define IS_COMPANION		1
	#define HAS_ORIENT		0
	#define ao_storage_pos_t	uint16_t
	#define HAS_ICP3_COUNT		0

	#define AVR_VCC_5V	       	0
	#define AVR_VCC_3V3		1
	#define AVR_CLOCK		8000000UL

	#define SPI_SLAVE_CS_PORT	PORTB
	#define SPI_SLAVE_CS_PIN	PINB
	#define SPI_SLAVE_CS_PIN_NO	PINB0

	#define SPI_SLAVE_PIN_0_3	1
	#define SPI_SLAVE_PIN_2_5	0

	#define AO_PYRO_NUM		8

	#define AO_PYRO_PORT_0	B
	#define AO_PYRO_PIN_0	5

	#define AO_PYRO_PORT_1	B
	#define AO_PYRO_PIN_1	6

	#define AO_PYRO_PORT_2	B
	#define AO_PYRO_PIN_2	7

	#define AO_PYRO_PORT_3	C
	#define AO_PYRO_PIN_3	6

	#define AO_PYRO_PORT_4	C
	#define AO_PYRO_PIN_4	7

	#define AO_PYRO_PORT_5	D
	#define AO_PYRO_PIN_5	5

	#define AO_PYRO_PORT_6	D
	#define AO_PYRO_PIN_6	3

	#define AO_PYRO_PORT_7	D
	#define AO_PYRO_PIN_7	2

#endif

#define AO_M25_SPI_CS_PORT	SPI_CS_PORT
#define AO_M25_SPI_CS_MASK	M25_CS_MASK

#define AO_TELESCIENCE_NUM_ADC	12

struct ao_adc {
	uint16_t	adc[AO_TELESCIENCE_NUM_ADC];	/* samples */
};

#define AO_DATA_RING	16

#endif /* _AO_PINS_H_ */

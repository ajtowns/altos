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
	#define USE_SERIAL_STDIN	1
	#define HAS_USB			1
	#define PACKET_HAS_SLAVE	0
	#define HAS_SERIAL_1		1
	#define TEENSY			1
	#define AVR_VCC_5V	       	1
	#define AVR_VCC_3V3		0
	#define AVR_CLOCK		16000000UL
	#define HAS_BEEP		0
#endif

#ifdef TELESCIENCE
	#define LEDS_AVAILABLE		0
	#define HAS_USB			1
	#define HAS_LOG			1
	#define TEENSY			0
	#define USE_SERIAL_STDIN	1
	#define HAS_SERIAL_1		1
	#define HAS_USB			1
	#define HAS_ADC			1
	#define PACKET_HAS_SLAVE	0
	#define HAS_BEEP		0

	#define AVR_VCC_5V	       	0
	#define AVR_VCC_3V3		1
	#define AVR_CLOCK		8000000UL

	#define SPI_CS_PORT		PORTE
	#define SPI_CS_DIR		DDRE
	#define M25_CS_MASK		(1 << PORTE6)
	#define M25_MAX_CHIPS		1

	#define SPI_SLAVE_CS_PORT	PORTB
	#define SPI_SLAVE_CS_PIN	PINB
	#define SPI_SLAVE_CS_PIN_NO	PINB0

	#define SPI_SLAVE_PIN_0_3	1
	#define SPI_SLAVE_PIN_2_5	0
#endif

#endif /* _AO_PINS_H_ */

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
#include <avr/pgmspace.h>

#define AO_LED_ORANGE		(1<<4)
#define AO_LED_SERIAL		4
#define AO_LED_PANIC		AO_LED_ORANGE
#define AO_LED_REPORT		AO_LED_ORANGE
#define LEDS_AVAILABLE		(AO_LED_ORANGE)
#define USE_SERIAL_1_STDIN	0
#define HAS_USB			0
#define PACKET_HAS_SLAVE	0
#define HAS_SERIAL_1		0
#define HAS_TASK		0
#define HAS_MS5607		1
#define HAS_MS5611		0
#define HAS_EEPROM		0
#define HAS_BEEP		0
#define AVR_CLOCK		250000UL

/* SPI */
#define SPI_PORT		PORTB
#define SPI_PIN			PINB
#define SPI_DIR			DDRB
#define AO_MS5607_CS_PORT	PORTB
#define AO_MS5607_CS_PIN	3

/* MS5607 */
#define AO_MS5607_SPI_INDEX	0
#define AO_MS5607_MISO_PORT	PORTB
#define AO_MS5607_MISO_PIN	0
#define AO_MS5607_BARO_OVERSAMPLE	4096
#define AO_MS5607_TEMP_OVERSAMPLE	1024

/* I2C */
#define I2C_PORT		PORTB
#define I2C_PIN			PINB
#define I2C_DIR			DDRB
#define I2C_PIN_SCL		PINB2
#define I2C_PIN_SDA		PINB0

#define AO_CONST_ATTRIB		PROGMEM
typedef int32_t alt_t;
#define FETCH_ALT(o)		((alt_t) pgm_read_dword(&altitude_table[o]))

#define AO_ALT_VALUE(x)		((x) * (alt_t) 10)

/* Pressure change (in Pa) to detect boost */
#ifndef BOOST_DETECT
#define BOOST_DETECT		120	/* 10m at sea level, 12m at 2000m */
#endif

#endif /* _AO_PINS_H_ */

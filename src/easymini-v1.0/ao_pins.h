/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

#define HAS_BEEP		1
#define HAS_BATTERY_REPORT	1

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
#define AO_USB_PULLUP_PIN	20

#define PACKET_HAS_SLAVE	0

#define AO_LOG_FORMAT		AO_LOG_FORMAT_EASYMINI

/* USART */

#define HAS_SERIAL		0
#define USE_SERIAL_0_STDIN	1
#define SERIAL_0_18_19		1
#define SERIAL_0_14_15		0
#define SERIAL_0_17_18		0
#define SERIAL_0_26_27		0

/* SPI */

#define HAS_SPI_0		1
#define SPI_SCK0_P0_6		1
#define HAS_SPI_1		1
#define SPI_SCK1_P1_15		1
#define SPI_MISO1_P0_22		1
#define SPI_MOSI1_P0_21		1

/* M25 */

#define M25_MAX_CHIPS		1
#define AO_M25_SPI_CS_PORT	0
#define AO_M25_SPI_CS_MASK	(1 << 23)
#define AO_M25_SPI_BUS		1

/* MS5607 */

#define HAS_MS5607		1
#define HAS_MS5611		0
#define AO_MS5607_PRIVATE_PINS	0
#define AO_MS5607_CS_PORT	0
#define AO_MS5607_CS_PIN	7
#define AO_MS5607_CS_MASK	(1 << AO_MS5607_CS_PIN)
#define AO_MS5607_MISO_PORT	0
#define AO_MS5607_MISO_PIN	8
#define AO_MS5607_MISO_MASK	(1 << AO_MS5607_MISO_PIN)
#define AO_MS5607_SPI_INDEX	0

#define HAS_ACCEL		0
#define HAS_GPS			0
#define HAS_RADIO		0
#define HAS_FLIGHT		1
#define HAS_EEPROM		1
#define HAS_TELEMETRY		0
#define HAS_APRS		0
#define HAS_LOG			1
#define USE_INTERNAL_FLASH	0
#define HAS_IGNITE		1
#define HAS_IGNITE_REPORT	1

#define AO_DATA_RING		16

/*
 * ADC
 */

#define HAS_ADC			1

#define AO_NUM_ADC		3

#define AO_ADC_0		1
#define AO_ADC_1		1
#define AO_ADC_2		1

struct ao_adc {
	int16_t		sense_a;
	int16_t		sense_m;
	int16_t		v_batt;
};

/*
 * Igniter
 */

#define AO_IGNITER_CLOSED	400
#define AO_IGNITER_OPEN		60

#define AO_IGNITER_DROGUE_PORT	0
#define AO_IGNITER_DROGUE_PIN	2
#define AO_IGNITER_SET_DROGUE(v)	ao_gpio_set(AO_IGNITER_DROGUE_PORT, AO_IGNITER_DROGUE_PIN, AO_IGNITER_DROGUE, v)

#define AO_IGNITER_MAIN_PORT	0
#define AO_IGNITER_MAIN_PIN	3
#define AO_IGNITER_SET_MAIN(v)		ao_gpio_set(AO_IGNITER_MAIN_PORT, AO_IGNITER_MAIN_PIN, AO_IGNITER_MAIN, v)

#define AO_SENSE_DROGUE(p)	((p)->adc.sense_a)
#define AO_SENSE_MAIN(p)	((p)->adc.sense_m)

#define AO_ADC_DUMP(p) \
	printf("tick: %5u apogee: %5d main: %5d batt: %5d\n", \
	       (p)->tick, (p)->adc.sense_a, (p)->adc.sense_m, (p)->adc.v_batt)

/*
 * Voltage divider on ADC battery sampler
 */
#define AO_BATTERY_DIV_PLUS	100	/* 100k */
#define AO_BATTERY_DIV_MINUS	27	/* 27k */

/*
 * Voltage divider on ADC igniter samplers
 */
#define AO_IGNITE_DIV_PLUS	100	/* 100k */
#define AO_IGNITE_DIV_MINUS	27	/* 27k */

/*
 * ADC reference in decivolts
 */
#define AO_ADC_REFERENCE_DV	33

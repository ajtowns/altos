/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

#define AO_STACK_SIZE	448

#define IS_FLASH_LOADER		0

/* Crystal on the board */
#define AO_LPC_CLKIN	12000000

/* Main clock frequency. 48MHz for USB so we don't use the USB PLL */
#define AO_LPC_CLKOUT	48000000

/* System clock frequency */
#define AO_LPC_SYSCLK	24000000

#define HAS_SERIAL_0		1
#define SERIAL_0_18_19		1
#define USE_SERIAL_0_STDIN	0

#define ao_gps_getchar		ao_serial0_getchar
#define ao_gps_putchar		ao_serial0_putchar
#define ao_gps_set_speed	ao_serial0_set_speed
#define ao_gps_fifo		(ao_usart_rx_fifo)

#define HAS_EEPROM		1
#define USE_INTERNAL_FLASH	0
#define HAS_USB			1
#define HAS_BEEP		0
#define HAS_RADIO		1
#define HAS_TELEMETRY		1
#define HAS_RDF			1
#define HAS_APRS		1
#define HAS_RADIO_RECV		0

#define HAS_USB_PULLUP		1
#define AO_USB_PULLUP_PORT	0
#define AO_USB_PULLUP_PIN	7

/* Flash part */
#define HAS_SPI_0		1
#define SPI_SCK0_P0_6		1
#define SPI_0_OSPEEDR		AO_SPI_OSPEED_12MHz

/* Radio */
#define HAS_SPI_1		1
#define SPI_SCK1_P1_15		1
#define SPI_MISO1_P0_22		1
#define SPI_MOSI1_P0_21		1

#define HAS_GPS			1
#define HAS_FLIGHT		0
#define HAS_ADC			0
#define HAS_LOG			1
#define HAS_TRACKER		1

#define AO_CONFIG_DEFAULT_APRS_INTERVAL		0
#define AO_CONFIG_DEFAULT_RADIO_POWER		0xc0
#define AO_CONFIG_DEFAULT_FLIGHT_LOG_MAX	496 * 1024

/*
 * GPS
 */

#define AO_SERIAL_SPEED_UBLOX	AO_SERIAL_SPEED_9600

/*
 * Radio (cc115l)
 */

/* gets pretty close to 434.550 */

#define AO_RADIO_CAL_DEFAULT 	1095378

#define HAS_RADIO_POWER		0
#define AO_FEC_DEBUG		0
#define AO_CC115L_SPI_CS_PORT	0
#define AO_CC115L_SPI_CS_PIN	3
#define AO_CC115L_SPI_BUS	0

#define AO_CC115L_FIFO_INT_GPIO_IOCFG	CC115L_IOCFG2
#define AO_CC115L_FIFO_INT_PORT		0
#define AO_CC115L_FIFO_INT_PIN		20

#define AO_CC115L_DONE_INT_GPIO_IOCFG	CC115L_IOCFG0
#define AO_CC115L_DONE_INT_PORT		0
#define AO_CC115L_DONE_INT_PIN		2

/*
 * Flash (M25)
 */
#define M25_MAX_CHIPS		1
#define AO_M25_SPI_CS_PORT	0
#define AO_M25_SPI_CS_MASK	(1 << 23)
#define AO_M25_SPI_BUS		1

#define PACKET_HAS_SLAVE	0

#endif /* _AO_PINS_H_ */

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

#define HAS_TASK_QUEUE		1

/* 8MHz High speed external crystal */
#define AO_HSE			8000000

/* PLLVCO = 96MHz (so that USB will work) */
#define AO_PLLMUL		12
#define AO_RCC_CFGR_PLLMUL	(STM_RCC_CFGR_PLLMUL_12)

/* SYSCLK = 32MHz (no need to go faster than CPU) */
#define AO_PLLDIV		3
#define AO_RCC_CFGR_PLLDIV	(STM_RCC_CFGR_PLLDIV_3)

/* HCLK = 32MHz (CPU clock) */
#define AO_AHB_PRESCALER	1
#define AO_RCC_CFGR_HPRE_DIV	STM_RCC_CFGR_HPRE_DIV_1

/* Run APB1 at 16MHz (HCLK/2) */
#define AO_APB1_PRESCALER	2
#define AO_RCC_CFGR_PPRE1_DIV	STM_RCC_CFGR_PPRE2_DIV_2

/* Run APB2 at 16MHz (HCLK/2) */
#define AO_APB2_PRESCALER	2
#define AO_RCC_CFGR_PPRE2_DIV	STM_RCC_CFGR_PPRE2_DIV_2

#define HAS_SERIAL_1		1
#define USE_SERIAL_1_STDIN	0
#define SERIAL_1_PB6_PB7	1
#define SERIAL_1_PA9_PA10	0

#define HAS_SERIAL_2		1
#define USE_SERIAL_2_STDIN	0
#define SERIAL_2_PA2_PA3	1
#define SERIAL_2_PD5_PD6	0

#define HAS_SERIAL_3		0
#define USE_SERIAL_3_STDIN	0
#define SERIAL_3_PB10_PB11	0
#define SERIAL_3_PC10_PC11	1
#define SERIAL_3_PD8_PD9	0

#define ao_gps_getchar		ao_serial2_getchar
#define ao_gps_putchar		ao_serial2_putchar
#define ao_gps_set_speed	ao_serial2_set_speed
#define ao_gps_fifo		(ao_stm_usart2.rx_fifo)

#define HAS_EEPROM		1
#define USE_INTERNAL_FLASH	0
#define USE_EEPROM_CONFIG	1
#define USE_STORAGE_CONFIG	0

#define HAS_USB			1
#define HAS_BEEP		0
#define HAS_RADIO		1
#define HAS_TELEMETRY		1
#define HAS_APRS		1
#define HAS_RADIO_RECV		0

#define HAS_SPI_1		1
#define SPI_1_PA5_PA6_PA7	1	/* SD card */
#define SPI_1_PB3_PB4_PB5	0
#define SPI_1_PE13_PE14_PE15	0
#define SPI_1_OSPEEDR		STM_OSPEEDR_10MHz

#define SPI_1_PORT		(&stm_gpioa)
#define SPI_1_SCK_PIN		5
#define SPI_1_MISO_PIN		6
#define SPI_1_MOSI_PIN		7

#define HAS_SPI_2		1
#define SPI_2_PB13_PB14_PB15	1	/* CC115L */
#define SPI_2_PD1_PD3_PD4	0
#define SPI_2_OSPEEDR		STM_OSPEEDR_10MHz

#define SPI_2_PORT		(&stm_gpiob)
#define SPI_2_SCK_PIN		13
#define SPI_2_MISO_PIN		14
#define SPI_2_MOSI_PIN		15

#define HAS_I2C_1		0
#define I2C_1_PB8_PB9		1

#define HAS_I2C_2		0
#define I2C_2_PB10_PB11		1

#define PACKET_HAS_SLAVE	0
#define PACKET_HAS_MASTER	0

#define LOW_LEVEL_DEBUG		0

#define LED_PORT_0_ENABLE	STM_RCC_AHBENR_GPIOAEN
#define LED_PORT_0		(&stm_gpioa)
#define LED_PORT_0_MASK		(0xff)
#define LED_PORT_0_SHIFT	0
#define LED_PIN_RED		0
#define LED_PIN_GREEN		2
#define AO_LED_RED		(1 << LED_PIN_RED)
#define AO_LED_GREEN		(1 << LED_PIN_GREEN)

#define LEDS_AVAILABLE		(AO_LED_RED | AO_LED_GREEN)

#define HAS_GPS			1
#define HAS_FLIGHT		0
#define HAS_ADC			0
#define HAS_LOG			0

/*
 * Telemetry monitoring
 */
#define HAS_MONITOR		0
#define LEGACY_MONITOR		0
#define HAS_MONITOR_PUT		0
#define AO_MONITOR_LED		AO_LED_GREEN

/*
 * Radio (cc115l)
 */

/* gets pretty close to 434.550 */

#define AO_RADIO_CAL_DEFAULT 	0x10b6a5

#define HAS_RADIO_POWER		1

#define AO_FEC_DEBUG		0
#define AO_CC115L_SPI_CS_PORT	(&stm_gpiob)
#define AO_CC115L_SPI_CS_PIN	12
#define AO_CC115L_SPI_BUS	AO_SPI_2_PB13_PB14_PB15
#define AO_CC115L_SPI		stm_spi2

#define AO_CC115L_FIFO_INT_GPIO_IOCFG	CC115L_IOCFG2
#define AO_CC115L_FIFO_INT_PORT		(&stm_gpioa)
#define AO_CC115L_FIFO_INT_PIN		(9)

#define AO_CC115L_DONE_INT_GPIO_IOCFG	CC115L_IOCFG0
#define AO_CC115L_DONE_INT_PORT		(&stm_gpioa)
#define AO_CC115L_DONE_INT_PIN		(10)

#define HAS_RADIO_AMP		1

/*
 * Power amplifier (RFPA0133)
 */

#define AO_PA_POWER_GPIO	(&stm_gpiob)
#define AO_PA_POWER_PIN		1
#define AO_PA_GAIN_8_GPIO	(&stm_gpiob)
#define AO_PA_GAIN_8_PIN	10
#define AO_PA_GAIN_16_GPIO	(&stm_gpiob)
#define AO_PA_GAIN_16_PIN	11

/*
 * SD card
 */

#define AO_SDCARD_SPI_BUS	AO_SPI_1_PA5_PA6_PA7
#define AO_SDCARD_SPI_PORT	SPI_1_PORT
#define AO_SDCARD_SPI_SCK_PIN	SPI_1_SCK_PIN
#define AO_SDCARD_SPI_MISO_PIN	SPI_1_MISO_PIN
#define AO_SDCARD_SPI_MOSI_PIN	SPI_1_MOSI_PIN
#define AO_SDCARD_SPI_CS_PORT	(&stm_gpioa)
#define AO_SDCARD_SPI_CS_PIN	4
#define AO_SDCARD_SPI		stm_spi1

#endif /* _AO_PINS_H_ */

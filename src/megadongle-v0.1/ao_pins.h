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

#define HAS_SERIAL_1		0
#define USE_SERIAL_1_STDIN	0
#define SERIAL_1_PB6_PB7	0
#define SERIAL_1_PA9_PA10	1

#define HAS_SERIAL_2		0
#define USE_SERIAL_2_STDIN	0
#define SERIAL_2_PA2_PA3	0
#define SERIAL_2_PD5_PD6	0

#define HAS_SERIAL_3		0
#define USE_SERIAL_3_STDIN	0
#define SERIAL_3_PB10_PB11	0
#define SERIAL_3_PC10_PC11	1
#define SERIAL_3_PD8_PD9	0

#define ao_gps_getchar		ao_serial3_getchar
#define ao_gps_putchar		ao_serial3_putchar
#define ao_gps_set_speed	ao_serial3_set_speed

#define HAS_EEPROM		0
#define USE_INTERNAL_FLASH	0
#define HAS_USB			1
#define HAS_BEEP		0
#define HAS_RADIO		1
#define HAS_TELEMETRY		0

#define HAS_SPI_1		0
#define SPI_1_PA5_PA6_PA7	0	/* Barometer */
#define SPI_1_PB3_PB4_PB5	0
#define SPI_1_PE13_PE14_PE15	0	/* Accelerometer */

#define HAS_SPI_2		1
#define SPI_2_PB13_PB14_PB15	1	/* Flash, Companion */
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
#define PACKET_HAS_MASTER	1

#define LOW_LEVEL_DEBUG		0

#define LED_PORT_0_ENABLE	STM_RCC_AHBENR_GPIOAEN
#define LED_PORT_1_ENABLE	STM_RCC_AHBENR_GPIOBEN
#define LED_PORT_0		(&stm_gpioa)
#define LED_PORT_0_MASK		(0xff)
#define LED_PORT_0_SHIFT	0
#define LED_PORT_1_MASK		(0xff00)
#define LED_PORT_1_SHIFT	0
#define LED_PORT_1		(&stm_gpiob)
#define LED_PIN_RED		1
#define LED_PIN_GREEN		12
#define AO_LED_RED		(1 << LED_PIN_RED)
#define AO_LED_GREEN		(1 << LED_PIN_GREEN)

#define LEDS_AVAILABLE		(AO_LED_RED | AO_LED_GREEN)

#define HAS_GPS			0
#define HAS_FLIGHT		0
#define HAS_ADC			0
#define HAS_LOG			0

/*
 * Telemetry monitoring
 */
#define HAS_MONITOR		1
#define LEGACY_MONITOR		0
#define HAS_MONITOR_PUT		1
#define AO_MONITOR_LED		AO_LED_GREEN

/*
 * Radio (cc1120)
 */

/* gets pretty close to 434.550 */

#define AO_RADIO_CAL_DEFAULT 	0x6ca333

#define AO_FEC_DEBUG		0
#define AO_CC1120_SPI_CS_PORT	(&stm_gpioa)
#define AO_CC1120_SPI_CS_PIN	0
#define AO_CC1120_SPI_BUS	AO_SPI_2_PB13_PB14_PB15
#define AO_CC1120_SPI		stm_spi2

#define AO_CC1120_INT_PORT	(&stm_gpioc)
#define AO_CC1120_INT_PIN	13

#define AO_CC1120_MCU_WAKEUP_PORT	(&stm_gpioc)
#define AO_CC1120_MCU_WAKEUP_PIN	(0)

#define AO_CC1120_INT_GPIO	2
#define AO_CC1120_INT_GPIO_IOCFG	CC1120_IOCFG2

#define AO_CC1120_MARC_GPIO	3
#define AO_CC1120_MARC_GPIO_IOCFG	CC1120_IOCFG3

/*
 * Profiling Viterbi decoding
 */

#ifndef AO_PROFILE
#define AO_PROFILE	       	0
#endif

#endif /* _AO_PINS_H_ */

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
#define SERIAL_1_PB6_PB7	0
#define SERIAL_1_PA9_PA10	1

#define HAS_SERIAL_2		0
#define USE_SERIAL_2_STDIN	0
#define SERIAL_2_PA2_PA3	0
#define SERIAL_2_PD5_PD6	0

#define HAS_SERIAL_3		1
#define USE_SERIAL_3_STDIN	0
#define SERIAL_3_PB10_PB11	0
#define SERIAL_3_PC10_PC11	1
#define SERIAL_3_PD8_PD9	0

#define ao_gps_getchar		ao_serial3_getchar
#define ao_gps_putchar		ao_serial3_putchar
#define ao_gps_set_speed	ao_serial3_set_speed

#define HAS_EEPROM		1
#define USE_INTERNAL_FLASH	0
#define HAS_USB			1
#define HAS_BEEP		1

#define HAS_SPI_1		1
#define SPI_1_PA5_PA6_PA7	1
#define SPI_1_PB3_PB4_PB5	0
#define SPI_1_PE13_PE14_PE15	0

#define HAS_SPI_2		1
#define SPI_2_PB13_PB14_PB15	1
#define SPI_2_PD1_PD3_PD4	0

#define HAS_I2C_1		1
#define I2C_1_PB8_PB9		1

#define HAS_I2C_2		1
#define I2C_2_PB10_PB11		1

#define PACKET_HAS_SLAVE	1

#define LOW_LEVEL_DEBUG		1

#define LED_PORT_ENABLE		STM_RCC_AHBENR_GPIOCEN
#define LED_PORT		stm_gpioc
#define LED_PIN_RED		8
#define LED_PIN_GREEN		9
#define AO_LED_RED		(1 << LED_PIN_RED)
#define AO_LED_GREEN		(1 << LED_PIN_GREEN)

#define LEDS_AVAILABLE		(AO_LED_RED | AO_LED_GREEN)

#define HAS_ADC			1

#define AO_ADC_RING		32
#define AO_ADC_NUM_SENSE	6

struct ao_adc {
	uint16_t		tick;
	int16_t			sense[AO_ADC_NUM_SENSE];
	int16_t			v_batt;
	int16_t			v_pbatt;
	int16_t			temp;
};

#define AO_ADC_SENSE_A		0
#define AO_ADC_SENSE_A_PORT	stm_gpioa
#define AO_ADC_SENSE_A_PIN	0

#define AO_ADC_SENSE_B		1
#define AO_ADC_SENSE_B_PORT	stm_gpioa
#define AO_ADC_SENSE_B_PIN	1

#define AO_ADC_SENSE_C		2
#define AO_ADC_SENSE_C_PORT	stm_gpioa
#define AO_ADC_SENSE_C_PIN	2

#define AO_ADC_SENSE_D		3
#define AO_ADC_SENSE_D_PORT	stm_gpioa
#define AO_ADC_SENSE_D_PIN	3

#define AO_ADC_SENSE_E		4
#define AO_ADC_SENSE_E_PORT	stm_gpioa
#define AO_ADC_SENSE_E_PIN	4

#define AO_ADC_SENSE_F		22
#define AO_ADC_SENSE_F_PORT	stm_gpioe
#define AO_ADC_SENSE_F_PIN	7

#define AO_ADC_V_BATT		8
#define AO_ADC_V_BATT_PORT	stm_gpiob
#define AO_ADC_V_BATT_PIN	0

#define AO_ADC_V_PBATT		9
#define AO_ADC_V_PBATT_PORT	stm_gpiob
#define AO_ADC_V_PBATT_PIN	1

#define AO_ADC_TEMP		16

#define AO_ADC_RCC_AHBENR	((1 << STM_RCC_AHBENR_GPIOAEN) | \
				 (1 << STM_RCC_AHBENR_GPIOEEN) | \
				 (1 << STM_RCC_AHBENR_GPIOBEN))

#define AO_NUM_ADC_PIN		(AO_ADC_NUM_SENSE + 2)

#define AO_ADC_PIN0_PORT	AO_ADC_SENSE_A_PORT
#define AO_ADC_PIN0_PIN		AO_ADC_SENSE_A_PIN
#define AO_ADC_PIN1_PORT	AO_ADC_SENSE_B_PORT
#define AO_ADC_PIN1_PIN		AO_ADC_SENSE_B_PIN
#define AO_ADC_PIN2_PORT	AO_ADC_SENSE_C_PORT
#define AO_ADC_PIN2_PIN		AO_ADC_SENSE_C_PIN
#define AO_ADC_PIN3_PORT	AO_ADC_SENSE_D_PORT
#define AO_ADC_PIN3_PIN		AO_ADC_SENSE_D_PIN
#define AO_ADC_PIN4_PORT	AO_ADC_SENSE_E_PORT
#define AO_ADC_PIN4_PIN		AO_ADC_SENSE_E_PIN
#define AO_ADC_PIN5_PORT	AO_ADC_SENSE_F_PORT
#define AO_ADC_PIN5_PIN		AO_ADC_SENSE_F_PIN
#define AO_ADC_PIN6_PORT	AO_ADC_V_BATT_PORT
#define AO_ADC_PIN6_PIN		AO_ADC_V_BATT_PIN
#define AO_ADC_PIN7_PORT	AO_ADC_V_PBATT_PORT
#define AO_ADC_PIN7_PIN		AO_ADC_V_PBATT_PIN

#define AO_NUM_ADC	       	(AO_ADC_NUM_SENSE + 3)

#define AO_ADC_SQ1		AO_ADC_SENSE_A
#define AO_ADC_SQ2		AO_ADC_SENSE_B
#define AO_ADC_SQ3		AO_ADC_SENSE_C
#define AO_ADC_SQ4		AO_ADC_SENSE_D
#define AO_ADC_SQ5		AO_ADC_SENSE_E
#define AO_ADC_SQ6		AO_ADC_SENSE_F
#define AO_ADC_SQ7		AO_ADC_V_BATT
#define AO_ADC_SQ8		AO_ADC_V_PBATT
#define AO_ADC_SQ9		AO_ADC_TEMP

/*
 * Pressure sensor settings
 */
#define AO_MS5607_CS_GPIO	stm_gpioc
#define AO_MS5607_CS		4
#define AO_MS5607_CS_MASK	(1 << AO_MS5607_CS)
#define AO_MS5607_SPI_INDEX	(STM_SPI_INDEX(1))

/*
 * SPI Flash memory
 */

#define M25_MAX_CHIPS		1
#define AO_M25_SPI_CS_PORT	stm_gpiod
#define AO_M25_SPI_CS_MASK	(1 << 3)
#define AO_M25_SPI_BUS		STM_SPI_INDEX(2)

/*
 * Radio (cc1120)
 */

#define AO_CC1120_SPI_CS_PORT	stm_gpioc
#define AO_CC1120_SPI_CS_PIN	5
#define AO_CC1120_SPI_BUS	STM_SPI_INDEX(2)

#define AO_CC1120_INT_PORT	stm_gpioc
#define AO_CC1120_INT_PIN	14

#define AO_CC1120_INT_GPIO	2

/*
 * Mag sensor (hmc5883)
 */

#define AO_HMC5883_INT_PORT	stm_gpioc
#define AO_HMC5883_INT_PIN	12
#define AO_HMC5883_I2C_INDEX	STM_SPI_INDEX(1)

/*
 * mpu6000
 */

#define AO_MPU6000_INT_PORT	stm_gpioc
#define AO_MPU6000_INT_PIN	13
#define AO_MPU6000_I2C_INDEX	STM_SPI_INDEX(1)

#endif /* _AO_PINS_H_ */

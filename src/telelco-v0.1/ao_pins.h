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

#define HAS_EEPROM		1
#define USE_INTERNAL_FLASH	1
#define HAS_USB			1
#define HAS_BEEP		1
#define HAS_RADIO		1
#define HAS_TELEMETRY		0
#define HAS_AES			1

#define HAS_SPI_1		0
#define SPI_1_PA5_PA6_PA7	0
#define SPI_1_PB3_PB4_PB5	0
#define SPI_1_PE13_PE14_PE15	0

#define HAS_SPI_2		1	/* CC1111 */
#define SPI_2_PB13_PB14_PB15	1
#define SPI_2_PD1_PD3_PD4	0
#define SPI_2_GPIO		(&stm_gpiob)
#define SPI_2_SCK		13
#define SPI_2_MISO		14
#define SPI_2_MOSI		15
#define SPI_2_OSPEEDR		STM_OSPEEDR_10MHz

#define HAS_I2C_1		0

#define HAS_I2C_2		0

#define PACKET_HAS_SLAVE	0
#define PACKET_HAS_MASTER	0

/*
 * Radio is a cc1111 connected via SPI
 */
#define AO_RADIO_CAL_DEFAULT	1186611

#define AO_RADIO_SPI_BUS	AO_SPI_2_PB13_PB14_PB15
#define AO_RADIO_CS_PORT	(&stm_gpiob)
#define AO_RADIO_CS_PIN		12

#define AO_RADIO_INT_PORT	(&stm_gpioc)
#define AO_RADIO_INT_PIN	14

#define LOW_LEVEL_DEBUG		0

#define LED_PORT_ENABLE		STM_RCC_AHBENR_GPIOCEN
#define LED_PORT		(&stm_gpioc)
#define LED_PIN_RED		7
#define LED_PIN_GREEN		8
#define LED_PIN_CONTINUITY_3	9
#define LED_PIN_CONTINUITY_2	10
#define LED_PIN_CONTINUITY_1	11
#define LED_PIN_CONTINUITY_0	12
#define LED_PIN_REMOTE_ARM	13
#define AO_LED_RED		(1 << LED_PIN_RED)
#define AO_LED_GREEN		(1 << LED_PIN_GREEN)
#define AO_LED_CONTINUITY_3	(1 << LED_PIN_CONTINUITY_3)
#define AO_LED_CONTINUITY_2	(1 << LED_PIN_CONTINUITY_2)
#define AO_LED_CONTINUITY_1	(1 << LED_PIN_CONTINUITY_1)
#define AO_LED_CONTINUITY_0	(1 << LED_PIN_CONTINUITY_0)

#define AO_LED_CONTINUITY_NUM	4

#define AO_LED_REMOTE_ARM	(1 << LED_PIN_REMOTE_ARM)

#define LEDS_AVAILABLE		(AO_LED_RED |		\
				 AO_LED_GREEN |		\
				 AO_LED_CONTINUITY_3 |	\
				 AO_LED_CONTINUITY_2 |	\
				 AO_LED_CONTINUITY_1 |	\
				 AO_LED_CONTINUITY_0 |	\
				 AO_LED_REMOTE_ARM)

/* LCD displays */

#define AO_LCD_STM_SEG_ENABLED_0 (		\
		(1 << 0) | /* PA1 */		\
		(1 << 1) | /* PA2 */		\
		(1 << 2) | /* PA3 */		\
		(1 << 3) | /* PA6 */		\
		(1 << 4) | /* PA7 */		\
		(1 << 5) | /* PB0 */		\
		(1 << 6) | /* PB1 */		\
		(1 << 7) | /* PB3 */		\
		(0 << 8) | /* PB4 */		\
		(0 << 9) | /* PB5 */		\
		(0 << 10) | /* PB10 */		\
		(0 << 11) | /* PB11 */		\
		(0 << 12) | /* PB12 */		\
		(0 << 13) | /* PB13 */		\
		(0 << 14) | /* PB14 */		\
		(0 << 15) | /* PB15 */		\
		(0 << 16) | /* PB8 */		\
		(0 << 17) | /* PA15 */		\
		(0 << 18) | /* PC0 */		\
		(0 << 19) | /* PC1 */		\
		(0 << 20) | /* PC2 */		\
		(0 << 21) | /* PC3 */		\
		(0 << 22) | /* PC4 */		\
		(0 << 23) | /* PC5 */		\
		(0 << 24) | /* PC6 */		\
		(0 << 25) | /* PC7 */		\
		(0 << 26) | /* PC8 */		\
		(0 << 27) | /* PC9 */		\
		(0 << 28) | /* PC10 or PD8 */	\
		(0 << 29) | /* PC11 or PD9 */	\
		(0 << 30) | /* PC12 or PD10 */	\
		(0 << 31))  /* PD2 or PD11 */

#define AO_LCD_STM_SEG_ENABLED_1 (		\
		(0 << 0) | /* PD12 */		\
		(0 << 1) | /* PD13 */		\
		(0 << 2) | /* PD14 */		\
		(0 << 3) | /* PD15 */		\
		(0 << 4) | /* PE0 */		\
		(0 << 5) | /* PE1 */		\
		(0 << 6) | /* PE2 */		\
		(0 << 7))  /* PE3 */

#define AO_LCD_STM_COM_ENABLED (		\
		(1 << 0) | /* PA8 */		\
		(1 << 1) | /* PA9 */		\
		(1 << 2) | /* PA10 */		\
		(0 << 3) | /* PB9 */		\
		(0 << 4) | /* PC10 */		\
		(0 << 5) | /* PC11 */		\
		(0 << 6)) /* PC12 */

#define AO_LCD_28_ON_C	0

#define AO_LCD_DUTY	STM_LCD_CR_DUTY_1_4

#define AO_SEGMENT_0		0
#define AO_SEGMENT_1		5
#define AO_SEGMENT_2		1
#define AO_SEGMENT_3		6
#define AO_SEGMENT_4		4
#define AO_SEGMENT_5		2
#define AO_SEGMENT_6		3
#define AO_SEGMENT_7		7

/*
 * Use event queue for input devices
 */

#define AO_EVENT		1

/*
 * Knobs
 */

#define AO_QUADRATURE_COUNT	2
#define AO_QUADRATURE_MODE	0

#define AO_QUADRATURE_0_PORT	&stm_gpioc
#define AO_QUADRATURE_0_A	3
#define AO_QUADRATURE_0_B	2

#define AO_QUADRATURE_PAD	0

#define AO_QUADRATURE_1_PORT	&stm_gpioc
#define AO_QUADRATURE_1_A	1
#define AO_QUADRATURE_1_B	0

#define AO_QUADRATURE_BOX	1

/*
 * Buttons
 */

#define AO_BUTTON_COUNT		2
#define AO_BUTTON_MODE		AO_EXTI_MODE_PULL_UP

#define AO_BUTTON_0_PORT	&stm_gpioc
#define AO_BUTTON_0		4

#define AO_BUTTON_ARM		0

#define AO_BUTTON_1_PORT	&stm_gpioc
#define AO_BUTTON_1		5

#define AO_BUTTON_FIRE		1

#endif /* _AO_PINS_H_ */

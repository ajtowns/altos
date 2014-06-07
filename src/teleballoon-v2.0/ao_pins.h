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

#define HAS_SERIAL_3		1
#define USE_SERIAL_3_STDIN	0
#define SERIAL_3_PB10_PB11	1
#define SERIAL_3_PC10_PC11	0
#define SERIAL_3_PD8_PD9	0

#define AO_CONFIG_DEFAULT_FLIGHT_LOG_MAX	(512 * 1024)
#define AO_CONFIG_MAX_SIZE			1024
#define LOG_ERASE_MARK				0x55
#define LOG_MAX_ERASE				128

#define HAS_EEPROM		1
#define USE_INTERNAL_FLASH	0
#define USE_EEPROM_CONFIG	1
#define USE_STORAGE_CONFIG	0
#define HAS_USB			1
#define HAS_BEEP		1
#define HAS_BATTERY_REPORT	1
#define BEEPER_CHANNEL		4
#define HAS_RADIO		1
#define HAS_TELEMETRY		1
#define HAS_APRS		1

#define HAS_SPI_1		1
#define SPI_1_PA5_PA6_PA7	1	/* Barometer */
#define SPI_1_PB3_PB4_PB5	1	/* Accelerometer */
#define SPI_1_PE13_PE14_PE15	0
#define SPI_1_OSPEEDR		STM_OSPEEDR_10MHz

#define HAS_SPI_2		1
#define SPI_2_PB13_PB14_PB15	1	/* Flash, Companion, Radio */
#define SPI_2_PD1_PD3_PD4	0
#define SPI_2_OSPEEDR		STM_OSPEEDR_10MHz

#define SPI_2_PORT		(&stm_gpiob)
#define SPI_2_SCK_PIN		13
#define SPI_2_MISO_PIN		14
#define SPI_2_MOSI_PIN		15

#define HAS_I2C_1		0
#define I2C_1_PB8_PB9		0

#define HAS_I2C_2		0
#define I2C_2_PB10_PB11		0

#define PACKET_HAS_SLAVE	1
#define PACKET_HAS_MASTER	0

#define LOW_LEVEL_DEBUG		0

#define LED_PORT_ENABLE		STM_RCC_AHBENR_GPIOCEN
#define LED_PORT		(&stm_gpioc)
#define LED_PIN_RED		14
#define LED_PIN_GREEN		15
#define AO_LED_RED		(1 << LED_PIN_RED)
#define AO_LED_GREEN		(1 << LED_PIN_GREEN)

#define LEDS_AVAILABLE		(AO_LED_RED | AO_LED_GREEN)

#define HAS_GPS			1
#define HAS_FLIGHT		1
#define HAS_ADC			1
#define HAS_ADC_TEMP		1
#define HAS_LOG			1
#define AO_USB_FORCE_IDLE	0

/*
 * Igniter
 */

#define HAS_IGNITE		0
#define HAS_IGNITE_REPORT	0

#define AO_SENSE_DROGUE(p)	((p)->adc.sense_a)
#define AO_SENSE_MAIN(p)	((p)->adc.sense_m)
#define AO_IGNITER_CLOSED	400
#define AO_IGNITER_OPEN		60

/* Drogue */
#define AO_IGNITER_DROGUE_PORT	(&stm_gpioa)
#define AO_IGNITER_DROGUE_PIN	8

/* Main */
#define AO_IGNITER_MAIN_PORT	(&stm_gpioa)
#define AO_IGNITER_MAIN_PIN	9

#define AO_IGNITER_SET_DROGUE(v)	stm_gpio_set(AO_IGNITER_DROGUE_PORT, AO_IGNITER_DROGUE_PIN, v)
#define AO_IGNITER_SET_MAIN(v)		stm_gpio_set(AO_IGNITER_MAIN_PORT, AO_IGNITER_MAIN_PIN, v)

/*
 * ADC
 */
#define AO_DATA_RING		32
#define AO_ADC_NUM_SENSE	2

struct ao_adc {
	int16_t			sense_a;
	int16_t			sense_m;
	int16_t			v_batt;
	int16_t			temp;
};

#define AO_ADC_DUMP(p) \
	printf("tick: %5u drogue: %5d main: %5d batt: %5d\n", \
	       (p)->tick, \
	       (p)->adc.sense_a, (p)->adc.sense_m, \
	       (p)->adc.v_batt);

#define AO_ADC_SENSE_DROGUE	0
#define AO_ADC_SENSE_DROGUE_PORT	(&stm_gpioa)
#define AO_ADC_SENSE_DROGUE_PIN	0

#define AO_ADC_SENSE_MAIN	1
#define AO_ADC_SENSE_MAIN_PORT	(&stm_gpioa)
#define AO_ADC_SENSE_MAIN_PIN	1

#define AO_ADC_V_BATT		8
#define AO_ADC_V_BATT_PORT	(&stm_gpiob)
#define AO_ADC_V_BATT_PIN	0

#define AO_ADC_TEMP		16

#define AO_ADC_RCC_AHBENR	((1 << STM_RCC_AHBENR_GPIOAEN) | \
				 (1 << STM_RCC_AHBENR_GPIOEEN) | \
				 (1 << STM_RCC_AHBENR_GPIOBEN))

#define AO_NUM_ADC_PIN		3

#define AO_ADC_PIN0_PORT	AO_ADC_SENSE_DROGUE_PORT
#define AO_ADC_PIN0_PIN		AO_ADC_SENSE_DROGUE_PIN
#define AO_ADC_PIN1_PORT	AO_ADC_SENSE_MAIN_PORT
#define AO_ADC_PIN1_PIN		AO_ADC_SENSE_MAIN_PIN
#define AO_ADC_PIN2_PORT	AO_ADC_V_BATT_PORT
#define AO_ADC_PIN2_PIN		AO_ADC_V_BATT_PIN

#define AO_NUM_ADC	       	(AO_NUM_ADC_PIN + 1)

#define AO_ADC_SQ1		AO_ADC_SENSE_DROGUE
#define AO_ADC_SQ2		AO_ADC_SENSE_MAIN
#define AO_ADC_SQ3		AO_ADC_V_BATT
#define AO_ADC_SQ4		AO_ADC_TEMP

/*
 * Voltage divider on ADC battery sampler
 */
#define AO_BATTERY_DIV_PLUS	56	/* 5.6k */
#define AO_BATTERY_DIV_MINUS	100	/* 10k */

/*
 * Voltage divider on ADC igniter samplers
 */
#define AO_IGNITE_DIV_PLUS	100	/* 100k */
#define AO_IGNITE_DIV_MINUS	27	/* 27k */

/*
 * ADC reference in decivolts
 */
#define AO_ADC_REFERENCE_DV	33

/*
 * GPS
 */

#define AO_SERIAL_SPEED_UBLOX	AO_SERIAL_SPEED_9600

#define ao_gps_getchar		ao_serial3_getchar
#define ao_gps_putchar		ao_serial3_putchar
#define ao_gps_set_speed	ao_serial3_set_speed
#define ao_gps_fifo		(ao_stm_usart3.rx_fifo)

/*
 * Pressure sensor settings
 */
#define HAS_MS5607		1
#define HAS_MS5611		0
#define AO_MS5607_PRIVATE_PINS	1
#define AO_MS5607_CS_PORT	(&stm_gpiob)
#define AO_MS5607_CS_PIN	12
#define AO_MS5607_CS_MASK	(1 << AO_MS5607_CS)
#define AO_MS5607_MISO_PORT	(&stm_gpioa)
#define AO_MS5607_MISO_PIN	6
#define AO_MS5607_MISO_MASK	(1 << AO_MS5607_MISO)
#define AO_MS5607_SPI_INDEX	AO_SPI_1_PA5_PA6_PA7

/*
 * SPI Flash memory
 */

#define M25_MAX_CHIPS		1
#define AO_M25_SPI_CS_PORT	(&stm_gpiob)
#define AO_M25_SPI_CS_MASK	(1 << 8)
#define AO_M25_SPI_BUS		AO_SPI_2_PB13_PB14_PB15

/*
 * Radio (cc1120)
 */

/* gets pretty close to 434.550 */

#define AO_RADIO_CAL_DEFAULT 	0x6ca333

#define AO_FEC_DEBUG		0
#define AO_CC1120_SPI_CS_PORT	(&stm_gpioa)
#define AO_CC1120_SPI_CS_PIN	2
#define AO_CC1120_SPI_BUS	AO_SPI_2_PB13_PB14_PB15
#define AO_CC1120_SPI		stm_spi2

#define AO_CC1120_INT_PORT		(&stm_gpioa)
#define AO_CC1120_INT_PIN		(3)
#define AO_CC1120_MCU_WAKEUP_PORT	(&stm_gpioa)
#define AO_CC1120_MCU_WAKEUP_PIN	(4)

#define AO_CC1120_INT_GPIO	2
#define AO_CC1120_INT_GPIO_IOCFG	CC1120_IOCFG2

#define AO_CC1120_MARC_GPIO	3
#define AO_CC1120_MARC_GPIO_IOCFG	CC1120_IOCFG3

#define HAS_BOOT_RADIO		0

#define HAS_HIGHG_ACCEL		0

/* Disable accelerometer for balloon mode */
   
#define HAS_ACCEL		0

/*
 * mma655x
 */

#define HAS_MMA655X		0
#define AO_MMA655X_SPI_INDEX	AO_SPI_1_PB3_PB4_PB5
#define AO_MMA655X_CS_PORT	(&stm_gpiob)
#define AO_MMA655X_CS_PIN	9
#define AO_MMA655X_INVERT	1

#define NUM_CMDS		16

/*
 * Companion
 */

#define AO_COMPANION_CS_PORT	(&stm_gpiob)
#define AO_COMPANION_CS_PIN	(6)
#define AO_COMPANION_SPI_BUS	AO_SPI_2_PB13_PB14_PB15

/*
 * Monitor
 */

#define HAS_MONITOR		0
#define LEGACY_MONITOR		0
#define HAS_MONITOR_PUT		1
#define AO_MONITOR_LED		0
#define HAS_RSSI		0

/*
 * Profiling Viterbi decoding
 */

#ifndef AO_PROFILE
#define AO_PROFILE	       	0
#endif

/*
 * Teleballoon-specific bits
 */

#define AO_TELEMETRY_INTERVAL_BALLOON	AO_MS_TO_TICKS(1000)

#define AO_SEND_METRUM		1

#endif /* _AO_PINS_H_ */

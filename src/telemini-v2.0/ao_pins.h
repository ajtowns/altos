/*
 * Copyright © 2010 Keith Packard <keithp@keithp.com>
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

#define HAS_RADIO	1

#define HAS_FLIGHT		1
#define HAS_USB			1
#define USB_FORCE_FLIGHT_IDLE	1
#define HAS_BEEP		1
#define HAS_GPS			0
#define HAS_SERIAL_1		0
#define HAS_EEPROM		1
#define HAS_LOG			1
#define USE_INTERNAL_FLASH	0
#define HAS_DBG			0
#define PACKET_HAS_SLAVE	1
#define USE_FAST_ASCENT_LOG	1

#define AO_LED_GREEN		1
#define AO_LED_RED		2
#define LEDS_AVAILABLE		(AO_LED_RED|AO_LED_GREEN)
#define HAS_EXTERNAL_TEMP	0
#define HAS_ACCEL		0
#define HAS_IGNITE		1
#define HAS_IGNITE_REPORT 1
#define HAS_MONITOR		0

/*
 * SPI
 */

#define SPI_CS_PORT		P1
#define SPI_CS_SEL		P1SEL
#define SPI_CS_DIR		P1DIR

/*
 * Flash
 */
#define AO_M25_SPI_CS_PORT	SPI_CS_PORT
#define AO_M25_SPI_CS_MASK	0x04	/* cs_flash is P1_2 */
#define M25_MAX_CHIPS		1

/*
 * MS5607
 */

#define HAS_MS5607		1
#define HAS_MS5611		0
#define AO_MS5607_PRIVATE_PINS	0
#define AO_MS5607_CS_PORT	P1
#define AO_MS5607_CS_PIN	3
#define AO_MS5607_CS		P1_3
#define AO_MS5607_CS_MASK	(1 << AO_MS5607_CS_PIN)
#define AO_MS5607_MISO_PORT	P0
#define AO_MS5607_MISO_PIN	2
#define AO_MS5607_MISO_MASK	(1 << AO_MS5607_MISO_PIN)
#define AO_MS5607_SPI_INDEX	0

/*
 * Igniters
 */
#define AO_IGNITER_PORT		P2
#define AO_IGNITER_DROGUE_PORT	AO_IGNITER_PORT
#define AO_IGNITER_DROGUE	P2_3
#define AO_IGNITER_MAIN		P2_4
#define AO_IGNITER_DIR		P2DIR
#define AO_IGNITER_DROGUE_BIT	(1 << 3)
#define AO_IGNITER_MAIN_BIT	(1 << 4)
#define AO_IGNITER_DROGUE_PIN	3
#define AO_IGNITER_MAIN_PIN	4

#define AO_IGNITER_DROGUE_PORT	AO_IGNITER_PORT
#define AO_IGNITER_MAIN_PORT	AO_IGNITER_PORT

/* test these values with real igniters */
#define AO_IGNITER_OPEN		1000
#define AO_IGNITER_CLOSED	7000
#define AO_IGNITER_FIRE_TIME	AO_MS_TO_TICKS(50)
#define AO_IGNITER_CHARGE_TIME	AO_MS_TO_TICKS(2000)

#define AO_SEND_MINI

/*
 * ADC
 */

#define HAS_ADC			1
#define AO_ADC_FIRST_PIN	0

struct ao_adc {
	int16_t		sense_a;	/* apogee continuity sense */
	int16_t		sense_m;	/* main continuity sense */
	int16_t		v_batt;		/* battery voltage */
};

#define ao_data_count	ao_adc_count

#define AO_SENSE_DROGUE(p)	((p)->adc.sense_a)
#define AO_SENSE_MAIN(p)	((p)->adc.sense_m)

#define AO_NUM_TASKS	10

#define AO_ADC_DUMP(p) \
	printf("tick: %5u apogee: %5d main: %5d batt: %5d\n", \
	       (p)->tick, (p)->adc.sense_a, (p)->adc.sense_m, (p)->adc.v_batt)

#define FETCH_ADC() 							\
	a = (uint8_t __xdata *) (&ao_data_ring[ao_data_head].adc); 	\
	switch (sequence) {						\
	case 4:								\
		a += 4;							\
		sequence = 0;						\
		break;							\
	case 1:								\
		a += 2;							\
		sequence = 4;						\
		break;							\
	case 0:								\
		sequence = 1;						\
		break;							\
	}								\
	if (sequence)							\
		;

#endif /* _AO_PINS_H_ */

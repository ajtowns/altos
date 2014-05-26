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

#ifndef _AO_MS5607_H_
#define _AO_MS5607_H_

#define AO_MS5607_RESET			0x1e

#define AO_MS5607_CONVERT_D1_256	0x40
#define AO_MS5607_CONVERT_D1_512	0x42
#define AO_MS5607_CONVERT_D1_1024	0x44
#define AO_MS5607_CONVERT_D1_2048	0x46
#define AO_MS5607_CONVERT_D1_4096	0x48

#define AO_MS5607_CONVERT_D2_256	0x50
#define AO_MS5607_CONVERT_D2_512	0x52
#define AO_MS5607_CONVERT_D2_1024	0x54
#define AO_MS5607_CONVERT_D2_2048	0x56
#define AO_MS5607_CONVERT_D2_4096	0x58

#define AO_MS5607_ADC_READ		0x00
#define AO_MS5607_PROM_READ(ad)		(0xA0 | ((ad) << 1))

struct ao_ms5607_prom {
	uint16_t	reserved;
	uint16_t	sens;
	uint16_t	off;
	uint16_t	tcs;
	uint16_t	tco;
	uint16_t	tref;
	uint16_t	tempsens;
	uint16_t	crc;
};

struct ao_ms5607_sample {
	uint32_t	pres;	/* raw 24 bit sensor */
	uint32_t	temp;	/* raw 24 bit sensor */
};

struct ao_ms5607_value {
	int32_t		pres;	/* in Pa * 10 */
	int32_t		temp;	/* in °C * 100 */
};

extern __xdata struct ao_ms5607_sample	ao_ms5607_current;
extern __xdata struct ao_ms5607_prom	ao_ms5607_prom;

void
ao_ms5607_setup(void);

void
ao_ms5607_init(void);

void
ao_ms5607_info(void);

void
ao_ms5607_sample(__xdata struct ao_ms5607_sample *sample);

void
ao_ms5607_convert(__xdata struct ao_ms5607_sample *sample,
		  __xdata struct ao_ms5607_value *value);

#endif /* _AO_MS5607_H_ */

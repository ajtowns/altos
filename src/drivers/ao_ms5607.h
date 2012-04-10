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

#endif /* _AO_MS5607_H_ */

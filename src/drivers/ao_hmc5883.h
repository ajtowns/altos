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

#ifndef _AO_HMC5883_H_
#define _AO_HMC5883_H_

#define HMC5883_ADDR_WRITE	0x3c
#define HMC5883_ADDR_READ	0x3d

#define HMC5883_CONFIG_A	0
#define HMC5883_CONFIG_B	1
#define HMC5883_MODE		2
#define HMC5883_X_MSB		3
#define HMC5883_X_LSB		4
#define HMC5883_Y_MSB		5
#define HMC5883_Y_LSB		6
#define HMC5883_Z_MSB		7
#define HMC5883_Z_LSB		8
#define HMC5883_STATUS		9
#define HMC5883_ID_A		10
#define HMC5883_ID_B		11
#define HMC5883_ID_C		12

void
ao_hmc5883_init(void);

#endif /* _AO_HMC5883_H_ */

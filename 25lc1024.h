/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

/* Defines for the 25LC1024 1Mbit SPI Bus Serial EEPROM */

#ifndef _25LC1024_H_
#define _25LC1024_H_

#define EE_READ		0x03
#define EE_WRITE	0x02
#define EE_WREN		0x06
#define EE_WRDI		0x04
#define EE_RDSR		0x05
#define EE_WRSR		0x01
#define EE_PE		0x42
#define EE_SE		0xd8
#define EE_CE		0xc7
#define EE_RDID		0xab
#define EE_DPD		0xb9

#define EE_STATUS_WIP	(1 << 0)
#define EE_STATUS_WEL	(1 << 1)
#define EE_STATUS_BP0	(1 << 2)
#define EE_STATUS_BP1	(1 << 3)
#define EE_STATUS_WPEN	(1 << 7)

#endif /* _25LC1024_H_ */

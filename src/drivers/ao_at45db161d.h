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

/* Defines for the Atmel AT45DB161D 16Mbit SPI Bus DataFlash® */

#ifndef _AT45DB161D_H_
#define _AT45DB161D_H_

/*
 * We reserve the last block on the device for
 * configuration space. Writes and reads in this
 * area return errors.
 */


#define FLASH_READ		0x03
#define FLASH_WRITE		0x82
#define FLASH_PAGE_ERASE	0x81
#define FLASH_READ_STATUS	0xd7
#define FLASH_SET_CONFIG	0x3d

#define FLASH_SET_512_BYTE_0	0x2a
#define FLASH_SET_512_BYTE_1	0x80
#define FLASH_SET_512_BYTE_2	0xa6

#define FLASH_STATUS_RDY		(1 << 7)
#define FLASH_STATUS_COMP		(1 << 6)
#define FLASH_STATUS_PROTECT		(1 << 1)
#define FLASH_STATUS_PAGESIZE_512	(1 << 0)

#endif /* _AT45DB161D_H_ */

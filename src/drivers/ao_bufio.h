/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_BUFIO_H_
#define _AO_BUFIO_H_

uint8_t *
ao_bufio_get(uint32_t block);

void
ao_bufio_put(uint8_t *buf, uint8_t write);

void
ao_bufio_flush_one(uint8_t *buf);

void
ao_bufio_flush(void);

void
ao_bufio_setup(void);

void
ao_bufio_init(void);

#endif /* _AO_BUFIO_H_ */

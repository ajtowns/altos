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

#ifndef _AO_STMLOAD_H_
#define _AO_STMLOAD_H_

struct sym {
	unsigned	addr;
	unsigned	default_addr;
	char		*name;
	int		required;
};

extern struct sym ao_symbols[];

extern int ao_num_symbols;
extern int ao_num_required_symbols;

void
ao_self_block_read(struct cc_usb *cc, uint32_t address, uint8_t block[256]);

void
ao_self_block_write(struct cc_usb *cc, uint32_t address, uint8_t block[256]);

struct hex_image *
ao_self_read(struct cc_usb *cc, uint32_t address, uint32_t length);

int
ao_self_write(struct cc_usb *cc, struct hex_image *image);

#endif /* _AO_STMLOAD_H_ */

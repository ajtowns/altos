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

#ifndef _AO_HEX_H_
#define _AO_HEX_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define AO_HEX_RECORD_NORMAL			0x00
#define AO_HEX_RECORD_EOF			0x01
#define AO_HEX_RECORD_EXTENDED_ADDRESS_4	0x02
#define AO_HEX_RECORD_EXTENDED_ADDRESS_8	0x04
#define AO_HEX_RECORD_SYMBOL			0xfe

/* Intel hex file format data
 */
struct ao_hex_record {
	uint8_t	length;
	uint16_t address;
	uint8_t type;
	uint8_t checksum;
	uint8_t data[0];
};

struct ao_hex_file {
	int			nrecord;
	struct ao_hex_record	*records[0];
};

struct ao_hex_image {
	uint32_t	address;
	uint32_t	length;
	uint8_t		data[0];
};

struct ao_sym {
	unsigned	addr;
	char		*name;
	bool		required;
	bool		found;
};

struct ao_hex_file *
ao_hex_file_read(FILE *file, char *name);

void
ao_hex_file_free(struct ao_hex_file *hex);

struct ao_hex_image *
ao_hex_image_create(struct ao_hex_file *hex);

void
ao_hex_image_free(struct ao_hex_image *image);

struct ao_hex_image *
ao_hex_load(char *filename, struct ao_sym **symbols, int *num_symbols);

int
ao_hex_image_equal(struct ao_hex_image *a, struct ao_hex_image *b);

bool
ao_hex_save(FILE *file, struct ao_hex_image *image,
	    struct ao_sym *symbols, int num_symbols);

#endif /* _AO_HEX_H_ */

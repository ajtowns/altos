/*
 * Copyright © 2008 Keith Packard <keithp@keithp.com>
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

#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ao-hex.h"
#include "ao-verbose.h"

struct ao_hex_input {
	FILE	*file;
	int	line;
	char	*name;
};

enum ao_hex_read_state {
	read_marker,
	read_length,
	read_address,
	read_type,
	read_data,
	read_checksum,
	read_newline,
	read_white,
	read_done,
};


static void
ao_hex_error(struct ao_hex_input *input, char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	fprintf(stderr, "Hex error %s:%d: ", input->name, input->line);
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

static void
ao_hex_free(struct ao_hex_record *record)
{
	if (!record) return;
	free(record);
}

static struct ao_hex_record *
ao_hex_alloc(uint8_t length)
{
	struct ao_hex_record *record;

	record = calloc(1, sizeof(struct ao_hex_record) + length);
	record->length = length;
	return record;
}

static int
ishex(char c)
{
	return isdigit(c) || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F');
}

static int
fromhex(char c)
{
	if (isdigit(c))
		return c - '0';
	if ('a' <= c && c <= 'f')
		return c - 'a' + 10;
	if ('A' <= c && c <= 'F')
		return c - 'A' + 10;
	abort();
	return 0;
}

static uint8_t
ao_hex_checksum(struct ao_hex_record *record)
{
	uint8_t	checksum = 0;
	int i;

	checksum += record->length;
	checksum += record->address >> 8;
	checksum += record->address & 0xff;
	checksum += record->type;
	for (i = 0; i < record->length; i++)
		checksum += record->data[i];
	return -checksum;
}

static struct ao_hex_record *
ao_hex_read_record(struct ao_hex_input *input)
{
	struct ao_hex_record *record = NULL;
	enum ao_hex_read_state state = read_marker;
	char c;
	int nhexbytes;
	uint32_t hex;
	uint32_t ndata;
	uint8_t checksum;

	while (state != read_done) {
		c = getc(input->file);
		if (c == EOF && state != read_white && state != read_marker) {
			ao_hex_error(input, "Unexpected EOF");
			goto bail;
		}
		if (c == ' ')
			continue;
		if (c == '\n')
			input->line++;
		switch (state) {
		case read_marker:
			if (c == EOF)
				return NULL;
			if (c != ':') {
				ao_hex_error(input, "Missing ':'");
				goto bail;
			}
			state = read_length;
			nhexbytes = 2;
			hex = 0;
			break;
		case read_length:
		case read_address:
		case read_type:
		case read_data:
		case read_checksum:
			if (!ishex(c)) {
				ao_hex_error(input, "Non-hex char '%c'",
						c);
				goto bail;
			}
			hex = hex << 4 | fromhex(c);
			--nhexbytes;
			if (nhexbytes != 0)
				break;

			switch (state) {
			case read_length:
				record = ao_hex_alloc(hex);
				if (!record) {
					ao_hex_error(input, "Out of memory");
					goto bail;
				}
				state = read_address;
				nhexbytes = 4;
				break;
			case read_address:
				record->address = hex;
				state = read_type;
				nhexbytes = 2;
				break;
			case read_type:
				record->type = hex;
				state = read_data;
				nhexbytes = 2;
				ndata = 0;
				break;
			case read_data:
				record->data[ndata] = hex;
				ndata++;
				nhexbytes = 2;
				break;
			case read_checksum:
				record->checksum = hex;
				state = read_newline;
				break;
			default:
				break;
			}
			if (state == read_data)
				if (ndata == record->length) {
					nhexbytes = 2;
					state = read_checksum;
				}
			hex = 0;
			break;
		case read_newline:
			if (c != '\n' && c != '\r') {
				ao_hex_error(input, "Missing newline");
				goto bail;
			}
			state = read_white;
			break;
		case read_white:
			if (!isspace(c)) {
				if (c == '\n')
					input->line--;
				if (c != EOF)
					ungetc(c, input->file);
				state = read_done;
			}
			break;
		case read_done:
			break;
		}
	}
	checksum = ao_hex_checksum(record);
	if (checksum != record->checksum) {
		ao_hex_error(input, "Invalid checksum (read 0x%02x computed 0x%02x)\n",
				record->checksum, checksum);
		goto bail;
	}
	return record;

bail:
	ao_hex_free(record);
	return NULL;
}

void
ao_hex_file_free(struct ao_hex_file *hex)
{
	int	i;

	if (!hex)
		return;
	for (i = 0; i < hex->nrecord; i++)
		ao_hex_free(hex->records[i]);
	free(hex);
}

struct ao_hex_file *
ao_hex_file_read(FILE *file, char *name)
{
	struct ao_hex_input input;
	struct ao_hex_file	*hex = NULL, *newhex;
	struct ao_hex_record *record;
	int srecord = 1;
	int done = 0;

	hex = calloc(sizeof (struct ao_hex_file) + sizeof (struct ao_hex_record *), 1);
	if (!hex)
		return NULL;
	input.name = name;
	input.line = 1;
	input.file = file;
	while (!done) {
		record = ao_hex_read_record(&input);
		if (!record) {
			if (feof(input.file)) {
				done = 1;
				break;
			} else
				goto bail;
		}
		if (hex->nrecord == srecord) {
			srecord *= 2;
			newhex = realloc(hex,
					 sizeof (struct ao_hex_file) +
					 srecord * sizeof (struct ao_hex_record *));
			if (!newhex)
				goto bail;
			hex = newhex;
		}
		hex->records[hex->nrecord++] = record;
	}
	return hex;

bail:
	ao_hex_file_free(hex);
	return NULL;
}

static struct ao_sym *
load_symbols(struct ao_hex_file *hex,
	     int *num_symbolsp)
{
	uint32_t		extended_addr;
	uint32_t		addr;
	int			i;
	struct ao_hex_record	*record;
	struct ao_sym		*symbols = NULL;
	struct ao_sym		*symbol;
	int			num_symbols = 0;
	int			size_symbols = 0;
	
	extended_addr = 0;
	for (i = 0; i < hex->nrecord; i++) {
		record = hex->records[i];
		switch (record->type) {
		case AO_HEX_RECORD_NORMAL:
			addr = extended_addr + record->address;
			break;
		case AO_HEX_RECORD_EOF:
			break;
		case AO_HEX_RECORD_EXTENDED_ADDRESS_4:
			if (record->length != 2)
				goto bail;
			extended_addr = ((record->data[0] << 8) | record->data[1]) << 4;
			break;
		case AO_HEX_RECORD_EXTENDED_ADDRESS_8:
			if (record->length != 2)
				goto bail;
			extended_addr = (record->data[0] << 24) | (record->data[1] << 16);
			break;
		case AO_HEX_RECORD_SYMBOL:
			addr = extended_addr + record->address;
			if (num_symbols == size_symbols) {
				struct ao_sym	*new_symbols;
				int		new_size;

				if (!size_symbols)
					new_size = 16;
				else
					new_size = size_symbols * 2;
				new_symbols = realloc(symbols, new_size * sizeof (struct ao_sym));
				if (!new_symbols)
					goto bail;

				symbols = new_symbols;
				size_symbols = new_size;
			}
			symbol = &symbols[num_symbols];
			memset(symbol, 0, sizeof (struct ao_sym));
			symbol->name = calloc(record->length + 1, 1);
			if (!symbol->name)
				goto bail;
			memcpy(symbol->name, record->data, record->length);
			symbol->addr = addr;
			ao_printf(AO_VERBOSE_EXE, "Add symbol %s: %08x\n", symbol->name, symbol->addr);
			num_symbols++;
			break;
		}
	}
	*num_symbolsp = num_symbols;
	return symbols;
bail:
	for (i = 0; i < num_symbols; i++)
		free(symbols[i].name);
	free(symbols);
	return NULL;
}

static void
ao_hex_record_set_checksum(struct ao_hex_record *record)
{
	uint8_t	cksum = 0;
	int i;

	cksum += record->length;
	cksum += record->address >> 8;
	cksum += record->address;
	cksum += record->type;
	for (i = 0; i < record->length; i++)
		cksum += record->data[i];

	record->checksum = -cksum;
}

struct ao_hex_image *
ao_hex_image_create(struct ao_hex_file *hex)
{
	struct ao_hex_image *image;
	struct ao_hex_record *record;
	int i;
	uint32_t addr;
	uint32_t base, bound;
	uint32_t offset;
	uint32_t extended_addr;

	int length;

	/* Find the address bounds of the file
	 */
	base = 0xffffffff;
	bound = 0x0;
	extended_addr = 0;
	for (i = 0; i < hex->nrecord; i++) {
		uint32_t r_bound;
		record = hex->records[i];
		switch (record->type) {
		case AO_HEX_RECORD_NORMAL:
			addr = extended_addr + record->address;
			r_bound = addr + record->length;
			if (addr < base)
				base = addr;
			if (r_bound > bound)
				bound = r_bound;
			break;
		case AO_HEX_RECORD_EOF:
			break;
		case AO_HEX_RECORD_EXTENDED_ADDRESS_4:
			if (record->length != 2)
				return NULL;
			extended_addr = ((record->data[0] << 8) | record->data[1]) << 4;
			break;
		case AO_HEX_RECORD_EXTENDED_ADDRESS_8:
			if (record->length != 2)
				return NULL;
			extended_addr = (record->data[0] << 24) | (record->data[1] << 16);
			break;
		case AO_HEX_RECORD_SYMBOL:
			break;
		}
	}
	length = bound - base;
	image = calloc(sizeof(struct ao_hex_image) + length, 1);
	if (!image)
		return NULL;
	image->address = base;
	image->length = length;
	memset(image->data, 0xff, length);
	extended_addr = 0;
	for (i = 0; i < hex->nrecord; i++) {
		record = hex->records[i];
		switch (record->type) {
		case AO_HEX_RECORD_NORMAL:
			addr = extended_addr + record->address;
			offset = addr - base;
			memcpy(image->data + offset, record->data, record->length);
			break;
		case AO_HEX_RECORD_EOF:
			break;
		case AO_HEX_RECORD_EXTENDED_ADDRESS_4:
			extended_addr = ((record->data[0] << 8) | record->data[1]) << 4;
			break;
		case AO_HEX_RECORD_EXTENDED_ADDRESS_8:
			extended_addr = (record->data[0] << 24) | (record->data[1] << 16);
			break;
		case AO_HEX_RECORD_SYMBOL:
			break;
		}
	}
	return image;
}

void
ao_hex_image_free(struct ao_hex_image *image)
{
	free(image);
}

int
ao_hex_image_equal(struct ao_hex_image *a, struct ao_hex_image *b)
{
	if (a->length != b->length)
		return 0;
	if (memcmp(a->data, b->data, a->length) != 0)
		return 0;
	return 1;
}

struct ao_hex_image *
ao_hex_load(char *filename, struct ao_sym **symbols, int *num_symbolsp)
{
	FILE			*file;
	struct ao_hex_file	*hex_file;
	struct ao_hex_image	*hex_image;

	file = fopen (filename, "r");
	if (!file)
		return NULL;
	
	hex_file = ao_hex_file_read(file, filename);
	fclose(file);
	if (!hex_file)
		return NULL;
	hex_image = ao_hex_image_create(hex_file);
	if (!hex_image)
		return NULL;

	if (symbols)
		*symbols = load_symbols(hex_file, num_symbolsp);

	ao_hex_file_free(hex_file);
	return hex_image;
}

#define BYTES_PER_RECORD	32

static struct ao_hex_file *
ao_hex_file_create(struct ao_hex_image *image, struct ao_sym *symbols, int num_symbols)
{
	/* split data into n-byte-sized chunks */
	uint32_t		data_records = (image->length + BYTES_PER_RECORD-1) / BYTES_PER_RECORD;
	/* extended address and data for each block, EOF, address and data for each symbol */
	uint32_t		total_records = data_records * 2 + 1 + num_symbols * 2;
	uint32_t		offset;
	uint32_t		address;
	uint32_t		length;
	char			*name;
	struct ao_hex_file	*hex_file;
	int			nrecord = 0;
	int			s;
	struct ao_hex_record	*record;

	hex_file = calloc(sizeof (struct ao_hex_file) + sizeof (struct ao_hex_record *) * total_records, 1);
	if (!hex_file)
		return NULL;

	/* Add the data
	 */
	for (offset = 0; offset < image->length; offset += BYTES_PER_RECORD) {
		uint32_t		address = image->address + offset;
		uint32_t		length = image->length - offset;

		if (length > BYTES_PER_RECORD)
			length = BYTES_PER_RECORD;

		record = calloc(sizeof (struct ao_hex_record) + 2, 1);
		record->type = AO_HEX_RECORD_EXTENDED_ADDRESS_8;
		record->address = 0;
		record->length = 2;
		record->data[0] = address >> 24;
		record->data[1] = address >> 16;
		ao_hex_record_set_checksum(record);

		hex_file->records[nrecord++] = record;

		record = calloc(sizeof (struct ao_hex_record) + length, 1);
		record->type = AO_HEX_RECORD_NORMAL;
		record->address = address;
		record->length = length;
		memcpy(record->data, image->data + offset, length);
		ao_hex_record_set_checksum(record);

		hex_file->records[nrecord++] = record;
	}

	/* Stick an EOF after the data
	 */
	record = calloc(sizeof (struct ao_hex_record), 1);
	record->type = AO_HEX_RECORD_EOF;
	record->address = 0;
	record->length = 0;
	record->data[0] = 0;
	record->data[1] = 0;
	ao_hex_record_set_checksum(record);

	hex_file->records[nrecord++] = record;
	
	/* Add the symbols
	 */

	for (s = 0; s < num_symbols; s++) {

		name = symbols[s].name;
		address = symbols[s].addr;
		length = strlen (name);

		record = calloc(sizeof (struct ao_hex_record) + 2, 1);
		record->type = AO_HEX_RECORD_EXTENDED_ADDRESS_8;
		record->address = 0;
		record->length = 2;
		record->data[0] = address >> 24;
		record->data[1] = address >> 16;
		ao_hex_record_set_checksum(record);

		hex_file->records[nrecord++] = record;

		record = calloc(sizeof (struct ao_hex_record) + length, 1);
		record->type = AO_HEX_RECORD_SYMBOL;
		record->address = address;
		record->length = length;
		memcpy(record->data, name, length);
		ao_hex_record_set_checksum(record);

		hex_file->records[nrecord++] = record;
	}

	hex_file->nrecord = nrecord;
	return hex_file;
}

static bool
ao_hex_write_record(FILE *file, struct ao_hex_record *record)
{
	int	i;

	fputc(':', file);
	fprintf(file, "%02x", record->length);
	fprintf(file, "%04x", record->address);
	fprintf(file, "%02x", record->type);
	for (i = 0; i < record->length; i++)
		fprintf(file, "%02x", record->data[i]);
	fprintf(file, "%02x", record->checksum);
	fputc('\n', file);
	return true;
}

bool
ao_hex_save(FILE *file, struct ao_hex_image *image,
	    struct ao_sym *symbols, int num_symbols)
{
	struct ao_hex_file	*hex_file;
	int			i;
	bool			ret = false;

	hex_file = ao_hex_file_create(image, symbols, num_symbols);
	if (!hex_file)
		goto create_failed;

	for (i = 0; i < hex_file->nrecord; i++) {
		if (!ao_hex_write_record(file, hex_file->records[i]))
			goto write_failed;
	}
	ret = true;
	
	if (fflush(file) != 0)
		ret = false;
write_failed:
	ao_hex_file_free(hex_file);
create_failed:
	return ret;
}

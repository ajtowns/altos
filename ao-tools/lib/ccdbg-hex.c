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

#include "ccdbg.h"
#include <stdarg.h>
#include <ctype.h>

struct hex_input {
	FILE	*file;
	int	line;
	char	*name;
};

enum hex_read_state {
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
ccdbg_hex_error(struct hex_input *input, char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	fprintf(stderr, "Hex error %s:%d: ", input->name, input->line);
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

static void
ccdbg_hex_free(struct hex_record *record)
{
	if (!record) return;
	free(record);
}

static struct hex_record *
ccdbg_hex_alloc(uint8_t length)
{
	struct hex_record *record;

	record = calloc(1, sizeof(struct hex_record) + length);
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
ccdbg_hex_checksum(struct hex_record *record)
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

static struct hex_record *
ccdbg_hex_read_record(struct hex_input *input)
{
	struct hex_record *record = NULL;
	enum hex_read_state state = read_marker;
	char c;
	int nhexbytes;
	uint32_t hex;
	uint32_t ndata;
	uint8_t checksum;

	while (state != read_done) {
		c = getc(input->file);
		if (c == EOF && state != read_white) {
			ccdbg_hex_error(input, "Unexpected EOF");
			goto bail;
		}
		if (c == ' ')
			continue;
		if (c == '\n')
			input->line++;
		switch (state) {
		case read_marker:
			if (c != ':') {
				ccdbg_hex_error(input, "Missing ':'");
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
				ccdbg_hex_error(input, "Non-hex char '%c'",
						c);
				goto bail;
			}
			hex = hex << 4 | fromhex(c);
			--nhexbytes;
			if (nhexbytes != 0)
				break;

			switch (state) {
			case read_length:
				record = ccdbg_hex_alloc(hex);
				if (!record) {
					ccdbg_hex_error(input, "Out of memory");
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
				ccdbg_hex_error(input, "Missing newline");
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
	checksum = ccdbg_hex_checksum(record);
	if (checksum != record->checksum) {
		ccdbg_hex_error(input, "Invalid checksum (read 0x%02x computed 0x%02x)\n",
				record->checksum, checksum);
		goto bail;
	}
	return record;

bail:
	ccdbg_hex_free(record);
	return NULL;
}

void
ccdbg_hex_file_free(struct hex_file *hex)
{
	int	i;

	if (!hex)
		return;
	for (i = 0; i < hex->nrecord; i++)
		ccdbg_hex_free(hex->records[i]);
	free(hex);
}

struct hex_file *
ccdbg_hex_file_read(FILE *file, char *name)
{
	struct hex_input input;
	struct hex_file	*hex = NULL, *newhex;
	struct hex_record *record;
	int srecord = 1;
	int done = 0;

	hex = calloc(sizeof (struct hex_file) + sizeof (struct hex_record *), 1);
	input.name = name;
	input.line = 1;
	input.file = file;
	while (!done) {
		record = ccdbg_hex_read_record(&input);
		if (!record)
			goto bail;
		if (hex->nrecord == srecord) {
			srecord *= 2;
			newhex = realloc(hex,
					 sizeof (struct hex_file) +
					 srecord * sizeof (struct hex_record *));
			if (!newhex)
				goto bail;
			hex = newhex;
		}
		hex->records[hex->nrecord++] = record;
		if (record->type == HEX_RECORD_EOF)
			done = 1;
	}
	return hex;

bail:
	ccdbg_hex_file_free(hex);
	return NULL;
}

struct hex_image *
ccdbg_hex_image_create(struct hex_file *hex)
{
	struct hex_image *image;
	struct hex_record *record;
	int i;
	uint32_t addr;
	uint32_t base, bound;
	uint32_t offset;
	uint32_t extended_addr;

	int length;

	base = 0xffffffff;
	bound = 0x0;
	extended_addr = 0;
	for (i = 0; i < hex->nrecord; i++) {
		uint32_t r_bound;
		record = hex->records[i];
		switch (record->type) {
		case 0:
			addr = extended_addr + record->address;
			r_bound = addr + record->length;
			if (addr < base)
				base = addr;
			if (r_bound > bound)
				bound = r_bound;
			break;
		case 1:
			break;
		case 2:
			if (record->length != 2)
				return NULL;
			extended_addr = ((record->data[0] << 8) | record->data[1]) << 4;
			break;
		case 4:
			if (record->length != 2)
				return NULL;
			extended_addr = ((record->data[0] << 8) | record->data[1]) << 16;
			break;
		}

	}
	length = bound - base;
	image = calloc(sizeof(struct hex_image) + length, 1);
	if (!image)
		return NULL;
	image->address = base;
	image->length = length;
	memset(image->data, 0xff, length);
	extended_addr = 0;
	for (i = 0; i < hex->nrecord; i++) {
		record = hex->records[i];
		switch (record->type) {
		case 0:
			addr = extended_addr + record->address;
			offset = addr - base;
			memcpy(image->data + offset, record->data, record->length);
			break;
		case 1:
			break;
		case 2:
			extended_addr = ((record->data[0] << 8) | record->data[1]) << 4;
			break;
		case 4:
			extended_addr = ((record->data[0] << 8) | record->data[1]) << 16;
			break;
		}
	}
	return image;
}

void
ccdbg_hex_image_free(struct hex_image *image)
{
	free(image);
}

int
ccdbg_hex_image_equal(struct hex_image *a, struct hex_image *b)
{
	if (a->length != b->length)
		return 0;
	if (memcmp(a->data, b->data, a->length) != 0)
		return 0;
	return 1;
}

struct hex_image *
ccdbg_hex_load(char *filename)
{
	FILE *file;
	struct hex_file	*hex_file;
	struct hex_image *hex_image;

	file = fopen (filename, "r");
	if (!file)
		return 0;
	
	hex_file = ccdbg_hex_file_read(file, filename);
	fclose(file);
	if (!hex_file)
		return 0;
	hex_image = ccdbg_hex_image_create(hex_file);
	if (!hex_image)
		return 0;
	ccdbg_hex_file_free(hex_file);
	return hex_image;
}

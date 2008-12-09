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

static void
get_bit(char *line, int i, char on, uint8_t bit, uint8_t *bits, uint8_t *masks)
{
	if (line[i] == on) {
		*bits |= bit;
		*masks |= bit;
		return;
	}
	if (line[i] == '.') {
		*masks |= bit;
		return;
	}
	if (line[i] == '-') {
		return;
	}
	fprintf(stderr, "bad line %s\n", line);
	exit (1);
}

static char
is_bit(uint8_t get, uint8_t mask, char on, uint8_t bit)
{
	if (mask&bit) {
		if (get&bit)
			return on;
		else
			return '.';
	} else
		return '-';
}

static uint8_t
ccdbg_write_read(struct ccdbg *dbg, uint8_t set, uint8_t mask)
{
	uint8_t	get = set;

	if (mask != (CC_DATA|CC_CLOCK|CC_RESET_N))
		get = ccdbg_read(dbg);
	ccdbg_write(dbg, mask, set);
	printf ("%c %c %c",
		is_bit(set, mask, 'C', CC_CLOCK),
		is_bit(set, mask, 'D', CC_DATA),
		is_bit(set, mask, 'R', CC_RESET_N));
	if (mask != (CC_DATA|CC_CLOCK|CC_RESET_N))
		printf(" -> %c %c %c",
		       is_bit(get, 0xf, 'C', CC_CLOCK),
		       is_bit(get, 0xf, 'D', CC_DATA),
		       is_bit(get, 0xf, 'R', CC_RESET_N));
	printf("\n");
	ccdbg_half_clock(dbg);
	return get;
}

static void
_ccdbg_debug_mode(struct ccdbg *dbg)
{
	printf ("#\n");
	printf ("# Debug mode\n");
	printf ("#\n");
	ccdbg_write_read(dbg, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA|CC_RESET_N);
	ccdbg_write_read(dbg,          CC_DATA           , CC_CLOCK|CC_DATA|CC_RESET_N);
	ccdbg_write_read(dbg, CC_CLOCK|CC_DATA           , CC_CLOCK|CC_DATA|CC_RESET_N);
	ccdbg_write_read(dbg,          CC_DATA           , CC_CLOCK|CC_DATA|CC_RESET_N);
	ccdbg_write_read(dbg, CC_CLOCK|CC_DATA           , CC_CLOCK|CC_DATA|CC_RESET_N);
	ccdbg_write_read(dbg,          CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA|CC_RESET_N);
}

static void
_ccdbg_reset(struct ccdbg *dbg)
{
	ccdbg_write_read(dbg, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA|CC_RESET_N);
	ccdbg_write_read(dbg, CC_CLOCK|CC_DATA           , CC_CLOCK|CC_DATA|CC_RESET_N);
	ccdbg_write_read(dbg, CC_CLOCK|CC_DATA           , CC_CLOCK|CC_DATA|CC_RESET_N);
	ccdbg_write_read(dbg, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA|CC_RESET_N);
}

static void
_ccdbg_send_bit(struct ccdbg *dbg, uint8_t bit)
{
	if (bit) bit = CC_DATA;
	ccdbg_write_read(dbg, CC_CLOCK|bit|CC_RESET_N, CC_CLOCK|CC_DATA|CC_RESET_N);
	ccdbg_write_read(dbg,          bit|CC_RESET_N, CC_CLOCK|CC_DATA|CC_RESET_N);
}

static void
_ccdbg_send_byte(struct ccdbg *dbg, uint8_t byte)
{
	int bit;
	printf ("#\n");
	printf ("# Send Byte 0x%02x\n", byte);
	printf ("#\n");
	for (bit = 7; bit >= 0; bit--) {
		_ccdbg_send_bit(dbg, (byte >> bit) & 1);
		if (bit == 3)
			printf ("\n");
	}
}

static void
_ccdbg_send_bits(struct ccdbg *dbg, int n, uint32_t bits)
{
	int bit;
	printf ("#\n");
	printf ("# Send %d bits 0x%08x\n", n, bits);
	printf ("#\n");
	for (bit = n - 1; bit >= 0; bit--) {
		_ccdbg_send_bit(dbg, (bits >> bit) & 1);
		if ((bit & 3) == 3)
			printf ("\n");
	}
}

static void
_ccdbg_print_bits(int n, uint32_t bits)
{
	int	bit;

	for (bit = n - 1; bit >= 0; bit--)
		printf ("%d", (bits >> bit) & 1);
}

static uint32_t
_ccdbg_read_bits(struct ccdbg *dbg, int bits)
{
	int		bit;
	uint32_t	val = 0;
	uint8_t		get;

	printf ("#\n");
	printf ("# Read %d bits\n", bits);
	printf ("#\n");
	for (bit = 0; bit < bits; bit++) {
		      ccdbg_write_read(dbg, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_RESET_N);
		get = ccdbg_write_read(dbg,          CC_DATA|CC_RESET_N, CC_CLOCK|CC_RESET_N);
		val <<= 1;
		if (get & CC_DATA)
			val |= 1;
		if ((bit & 3) == 3)
			printf ("\n");
	}
	printf ("#\n");
	printf ("# Read "); _ccdbg_print_bits(bits, val); printf ("\n");
	printf ("#\n");
	return val;
}

static int
_ccdbg_check_bits(uint32_t bits, uint8_t match)
{
	int	bit;

	for (bit = 0; bit < 24; bit++)
		if (((bits >> bit) & 0xff) == match)
			return 1;
	return 0;
}

static uint32_t
_ccdbg_play(struct ccdbg *dbg, int num_sync, uint32_t sync)
{
	uint32_t	bits;
	_ccdbg_debug_mode(dbg);
	_ccdbg_send_bits(dbg, num_sync, sync);
	_ccdbg_send_byte(dbg, CC_GET_CHIP_ID);
	bits = _ccdbg_read_bits(dbg, 16);
	_ccdbg_send_byte(dbg, CC_GET_CHIP_ID);
	bits = _ccdbg_read_bits(dbg, 16);
//	_ccdbg_send_byte(dbg, CC_READ_STATUS);
	_ccdbg_reset(dbg);
	if (_ccdbg_check_bits(bits, 0x11)) {
		printf("#\n#match with %d bits 0x%08x\n#\n", num_sync, sync);
		return 1;
	}
	return 0;
}

static int
_ccdbg_play_num(struct ccdbg *dbg, int num)
{
	uint32_t	sync;
	uint32_t	max;

	printf ("#\n#play %d\n#\n", num);
	max = (1 << num);
	for (sync = 0; sync < max; sync++)
		if (_ccdbg_play(dbg, num, sync))
			return 1;
	return 0;
}

static int
_ccdbg_play_many(struct ccdbg *dbg, int max)
{
	int	num;

	for (num = 0; num < max; num++)
		if (_ccdbg_play_num(dbg, num))
			return 1;
	return 0;
}

static void
ccdbg_manual(struct ccdbg *dbg, FILE *input)
{
	char	line[80];
	uint8_t	set, mask;

	while (fgets(line, sizeof line, input)) {
		if (line[0] == '#' || line[0] == '\n') {
			printf ("%s", line);
			continue;
		}
		set = 0;
		mask = 0;
		get_bit(line, 0, 'C', CC_CLOCK, &set, &mask);
		get_bit(line, 2, 'D', CC_DATA, &set, &mask);
		get_bit(line, 4, 'R', CC_RESET_N, &set, &mask);
		ccdbg_write_read(dbg, set, mask);
	}
}

int
main (int argc, char **argv)
{
	struct ccdbg	*dbg;
	uint8_t		status;
	uint16_t	chip_id;

	dbg = ccdbg_open("/dev/ttyUSB0");
	if (!dbg)
		exit (1);
#if 0	
	_ccdbg_play(dbg, 0, 0);
	_ccdbg_play_many(dbg, 8);
#endif
	ccdbg_manual(dbg, stdin);
#if 0
	ccdbg_debug_mode(dbg);
	status = ccdbg_read_status(dbg);
	printf("Status: 0x%02x\n", status);
	chip_id = ccdbg_get_chip_id(dbg);
	printf("Chip id: 0x%04x\n", chip_id);
	_ccdbg_reset(dbg);
#endif
	ccdbg_close(dbg);
	exit (0);
}

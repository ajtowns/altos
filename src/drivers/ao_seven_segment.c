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

#include <ao.h>
#include <ao_seven_segment.h>
#include <ao_lcd_stm.h>

/*
 *         0
 *	-------
 *     |       |  
 *   1 |       | 2
 *     |   3   |
 *      -------
 *     |       |
 *   4 |       | 5
 *     |   6   |
 *      -------
 *              [] 7
 *
 */

#ifndef SEVEN_SEGMENT_DEBUG
#define SEVEN_SEGMENT_DEBUG 0
#endif

static const uint8_t ao_segments[] = {
	(1 << 0) |
	(1 << 1) |
	(1 << 2) |
	(0 << 3) |
	(1 << 4) |
	(1 << 5) |
	(1 << 6),		/* 0 */

	(0 << 0) |
	(0 << 1) |
	(1 << 2) |
	(0 << 3) |
	(0 << 4) |
	(1 << 5) |
	(0 << 6),		/* 1 */

	(1 << 0) |
	(0 << 1) |
	(1 << 2) |
	(1 << 3) |
	(1 << 4) |
	(0 << 5) |
	(1 << 6),		/* 2 */

	(1 << 0) |
	(0 << 1) |
	(1 << 2) |
	(1 << 3) |
	(0 << 4) |
	(1 << 5) |
	(1 << 6),		/* 3 */

	(0 << 0) |
	(1 << 1) |
	(1 << 2) |
	(1 << 3) |
	(0 << 4) |
	(1 << 5) |
	(0 << 6),		/* 4 */

	(1 << 0) |
	(1 << 1) |
	(0 << 2) |
	(1 << 3) |
	(0 << 4) |
	(1 << 5) |
	(1 << 6),		/* 5 */

	(1 << 0) |
	(1 << 1) |
	(0 << 2) |
	(1 << 3) |
	(1 << 4) |
	(1 << 5) |
	(1 << 6),		/* 6 */

	(1 << 0) |
	(0 << 1) |
	(1 << 2) |
	(0 << 3) |
	(0 << 4) |
	(1 << 5) |
	(0 << 6),		/* 7 */

	(1 << 0) |
	(1 << 1) |
	(1 << 2) |
	(1 << 3) |
	(1 << 4) |
	(1 << 5) |
	(1 << 6),		/* 8 */

	(1 << 0) |
	(1 << 1) |
	(1 << 2) |
	(1 << 3) |
	(0 << 4) |
	(1 << 5) |
	(1 << 6),		/* 9 */

	(1 << 0) |
	(1 << 1) |
	(1 << 2) |
	(1 << 3) |
	(1 << 4) |
	(1 << 5) |
	(0 << 6),		/* A */

	(0 << 0) |
	(1 << 1) |
	(0 << 2) |
	(1 << 3) |
	(1 << 4) |
	(1 << 5) |
	(1 << 6),		/* b */

	(1 << 0) |
	(1 << 1) |
	(0 << 2) |
	(0 << 3) |
	(1 << 4) |
	(0 << 5) |
	(1 << 6),		/* c */

	(0 << 0) |
	(0 << 1) |
	(1 << 2) |
	(1 << 3) |
	(1 << 4) |
	(1 << 5) |
	(1 << 6),		/* d */

	(1 << 0) |
	(1 << 1) |
	(0 << 2) |
	(1 << 3) |
	(1 << 4) |
	(0 << 5) |
	(1 << 6),		/* E */

	(1 << 0) |
	(1 << 1) |
	(0 << 2) |
	(1 << 3) |
	(1 << 4) |
	(0 << 5) |
	(0 << 6),		/* F */
};

void
ao_seven_segment_set(uint8_t digit, uint8_t value)
{
	uint8_t	s;
	uint8_t	segments;

	if (value == AO_SEVEN_SEGMENT_CLEAR)
		segments = 0;
	else {
		segments = ao_segments[value & 0xf];

		/* Check for decimal point */
		if (value & 0x10)
			segments |= (1 << 7);
	}

	for (s = 0; s <= 7; s++)
		ao_lcd_set(digit, s, !!(segments & (1 << s)));
	ao_lcd_flush();
}

void
ao_seven_segment_clear(void)
{
	ao_lcd_clear();
}


#if SEVEN_SEGMENT_DEBUG
static void
ao_seven_segment_show(void)
{
	uint8_t	digit, value;
	ao_cmd_decimal();
	digit = ao_cmd_lex_i;
	ao_cmd_decimal();
	value = ao_cmd_lex_i;
	ao_seven_segment_set(digit, value);
}


static const struct ao_cmds ao_seven_segment_cmds[] = {
	{ ao_seven_segment_show,	"S <digit> <value>\0Set LCD digit" },
	{ 0, NULL },
};
#endif

void
ao_seven_segment_init(void)
{
#if SEVEN_SEGMENT_DEBUG
	ao_cmd_register(ao_seven_segment_cmds);
#endif
}

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

static const uint8_t ao_segments[] = {
	(1 << AO_SEGMENT_0) |
	(1 << AO_SEGMENT_1) |
	(1 << AO_SEGMENT_2) |
	(0 << AO_SEGMENT_3) |
	(1 << AO_SEGMENT_4) |
	(1 << AO_SEGMENT_5) |
	(1 << AO_SEGMENT_6),		/* 0 */

	(0 << AO_SEGMENT_0) |
	(0 << AO_SEGMENT_1) |
	(1 << AO_SEGMENT_2) |
	(0 << AO_SEGMENT_3) |
	(0 << AO_SEGMENT_4) |
	(1 << AO_SEGMENT_5) |
	(0 << AO_SEGMENT_6),		/* 1 */

	(1 << AO_SEGMENT_0) |
	(0 << AO_SEGMENT_1) |
	(1 << AO_SEGMENT_2) |
	(1 << AO_SEGMENT_3) |
	(1 << AO_SEGMENT_4) |
	(0 << AO_SEGMENT_5) |
	(1 << AO_SEGMENT_6),		/* 2 */

	(1 << AO_SEGMENT_0) |
	(0 << AO_SEGMENT_1) |
	(1 << AO_SEGMENT_2) |
	(1 << AO_SEGMENT_3) |
	(0 << AO_SEGMENT_4) |
	(1 << AO_SEGMENT_5) |
	(1 << AO_SEGMENT_6),		/* 3 */

	(0 << AO_SEGMENT_0) |
	(1 << AO_SEGMENT_1) |
	(1 << AO_SEGMENT_2) |
	(1 << AO_SEGMENT_3) |
	(0 << AO_SEGMENT_4) |
	(1 << AO_SEGMENT_5) |
	(0 << AO_SEGMENT_6),		/* 4 */

	(1 << AO_SEGMENT_0) |
	(1 << AO_SEGMENT_1) |
	(0 << AO_SEGMENT_2) |
	(1 << AO_SEGMENT_3) |
	(0 << AO_SEGMENT_4) |
	(1 << AO_SEGMENT_5) |
	(1 << AO_SEGMENT_6),		/* 5 */

	(1 << AO_SEGMENT_0) |
	(1 << AO_SEGMENT_1) |
	(0 << AO_SEGMENT_2) |
	(1 << AO_SEGMENT_3) |
	(1 << AO_SEGMENT_4) |
	(1 << AO_SEGMENT_5) |
	(1 << AO_SEGMENT_6),		/* 6 */

	(1 << AO_SEGMENT_0) |
	(0 << AO_SEGMENT_1) |
	(1 << AO_SEGMENT_2) |
	(0 << AO_SEGMENT_3) |
	(0 << AO_SEGMENT_4) |
	(1 << AO_SEGMENT_5) |
	(0 << AO_SEGMENT_6),		/* 7 */

	(1 << AO_SEGMENT_0) |
	(1 << AO_SEGMENT_1) |
	(1 << AO_SEGMENT_2) |
	(1 << AO_SEGMENT_3) |
	(1 << AO_SEGMENT_4) |
	(1 << AO_SEGMENT_5) |
	(1 << AO_SEGMENT_6),		/* 8 */

	(1 << AO_SEGMENT_0) |
	(1 << AO_SEGMENT_1) |
	(1 << AO_SEGMENT_2) |
	(1 << AO_SEGMENT_3) |
	(0 << AO_SEGMENT_4) |
	(1 << AO_SEGMENT_5) |
	(1 << AO_SEGMENT_6),		/* 9 */

	(1 << AO_SEGMENT_0) |
	(1 << AO_SEGMENT_1) |
	(1 << AO_SEGMENT_2) |
	(1 << AO_SEGMENT_3) |
	(1 << AO_SEGMENT_4) |
	(1 << AO_SEGMENT_5) |
	(0 << AO_SEGMENT_6),		/* A */

	(0 << AO_SEGMENT_0) |
	(1 << AO_SEGMENT_1) |
	(0 << AO_SEGMENT_2) |
	(1 << AO_SEGMENT_3) |
	(1 << AO_SEGMENT_4) |
	(1 << AO_SEGMENT_5) |
	(1 << AO_SEGMENT_6),		/* b */

	(1 << AO_SEGMENT_0) |
	(1 << AO_SEGMENT_1) |
	(0 << AO_SEGMENT_2) |
	(0 << AO_SEGMENT_3) |
	(1 << AO_SEGMENT_4) |
	(0 << AO_SEGMENT_5) |
	(1 << AO_SEGMENT_6),		/* c */

	(0 << AO_SEGMENT_0) |
	(0 << AO_SEGMENT_1) |
	(1 << AO_SEGMENT_2) |
	(1 << AO_SEGMENT_3) |
	(1 << AO_SEGMENT_4) |
	(1 << AO_SEGMENT_5) |
	(1 << AO_SEGMENT_6),		/* d */

	(1 << AO_SEGMENT_0) |
	(1 << AO_SEGMENT_1) |
	(0 << AO_SEGMENT_2) |
	(1 << AO_SEGMENT_3) |
	(1 << AO_SEGMENT_4) |
	(0 << AO_SEGMENT_5) |
	(1 << AO_SEGMENT_6),		/* E */

	(1 << AO_SEGMENT_0) |
	(1 << AO_SEGMENT_1) |
	(0 << AO_SEGMENT_2) |
	(1 << AO_SEGMENT_3) |
	(1 << AO_SEGMENT_4) |
	(0 << AO_SEGMENT_5) |
	(0 << AO_SEGMENT_6),		/* F */
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
			segments |= (1 << AO_SEGMENT_7);
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


#if 0
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
#if 0
	ao_cmd_register(ao_seven_segment_cmds);
#endif
}

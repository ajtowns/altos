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

/*
 * The provided 'calibration' value is
 * that needed to tune the radio to precisely 434550kHz.
 * Use that to 'walk' to the target frequency by following
 * a 'bresenham' line from 434550kHz to the target
 * frequency, and updating the radio setting along the way
 */

int32_t ao_freq_to_set(int32_t freq, int32_t cal) __reentrant
{
	static __pdata int32_t	set;
	static __pdata uint8_t	neg;
	static __pdata int32_t	error;

	set = 0;
	neg = 0;
	error = -434550 / 2;

	if ((freq -= 434550) < 0) {
		neg = 1;
		freq = -freq;
	}
	for (;;) {
		if (error > 0) {
			error -= 434550;
			set++;
		} else {
			error += cal;
			if (--freq < 0)
				break;
		}
	}
	if (neg)
		set = -set;
	return cal + set;
}

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

int32_t ao_freq_to_set(int32_t freq, int32_t cal) {
	__pdata int32_t	set = 0;
	uint8_t	neg = 0;
	__pdata int32_t	error = -434550 / 2;

	freq -= 434550;
	if (freq < 0) {
		neg = 1;
		freq = -freq;
	}
	for (;;) {
		if (freq == 0 && error <= 0)
			break;
		if (error > 0) {
			error -= 434550;
			set++;
		} else {
			error += cal;
			freq--;
		}
	}
	if (neg)
		set = -set;
	return cal + set;
}

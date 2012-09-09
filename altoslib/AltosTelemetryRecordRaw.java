/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

package org.altusmetrum.AltosLib;

public class AltosTelemetryRecordRaw extends AltosTelemetryRecord {
	int[]	bytes;
	int	serial;
	int	tick;
	int	type;

	long	received_time;

	public int int8(int off) {
		return AltosLib.int8(bytes, off + 1);
	}

	public int uint8(int off) {
		return AltosLib.uint8(bytes, off + 1);
	}

	public int int16(int off) {
		return AltosLib.int16(bytes, off + 1);
	}

	public int uint16(int off) {
		return AltosLib.uint16(bytes, off + 1);
	}

	public int uint32(int off) {
		return AltosLib.uint32(bytes, off + 1);
	}

	public int int32(int off) {
		return AltosLib.int32(bytes, off + 1);
	}

	public String string(int off, int l) {
		return AltosLib.string(bytes, off + 1, l);
	}

	public AltosTelemetryRecordRaw(int[] in_bytes) {
		bytes = in_bytes;
		serial = uint16(0);
		tick   = uint16(2);
		type   = uint8(4);
	}

	public AltosRecord update_state(AltosRecord previous) {
		AltosRecord	next;
		if (previous != null)
			next = previous.clone();
		else
			next = new AltosRecord();
		next.serial = serial;
		next.tick = tick;
		return next;
	}

	public long received_time() {
		return received_time;
	}
}

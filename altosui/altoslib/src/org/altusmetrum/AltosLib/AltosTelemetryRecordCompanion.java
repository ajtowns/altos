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

public class AltosTelemetryRecordCompanion extends AltosTelemetryRecordRaw {

	AltosRecordCompanion	companion;

	public AltosTelemetryRecordCompanion(int[] in_bytes) {
		super(in_bytes);

		int	off = 0;
		if (uint8(6) == 0)
			off = 1;
		int channels = uint8(7+off);

		if (off != 0 && channels >= 12)
			channels = 11;

		companion = new AltosRecordCompanion(channels);
		companion.tick		= tick;
		companion.board_id      = uint8(5);
		companion.update_period = uint8(6+off);
		for (int i = 0; i < companion.companion_data.length; i++)
			companion.companion_data[i] = uint16(8 + off + i * 2);
	}

	public AltosRecord update_state(AltosRecord previous) {
		AltosRecord	next = super.update_state(previous);

		next.companion = companion;
		next.seen |= AltosRecord.seen_sensor | AltosRecord.seen_temp_volt;

		companion.tick = tick;
		return next;
	}
}

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

public class AltosTelemetryRecordSatellite extends AltosTelemetryRecordRaw {
	int		channels;
	AltosGPSSat[]	sats;

	public AltosTelemetryRecordSatellite(int[] in_bytes) {
		super(in_bytes);

		channels = uint8(5);
		if (channels > 12)
			channels = 12;
		if (channels == 0)
			sats = null;
		else {
			sats = new AltosGPSSat[channels];
			for (int i = 0; i < channels; i++) {
				int	svid =  uint8(6 + i * 2 + 0);
				int	c_n_1 = uint8(6 + i * 2 + 1);
				sats[i] = new AltosGPSSat(svid, c_n_1);
			}
		}
	}

	public AltosRecord update_state(AltosRecord previous) {
		AltosRecord	next = super.update_state(previous);

		if (next.gps == null)
			next.gps = new AltosGPS();

		next.gps.cc_gps_sat = sats;

		return next;
	}
}

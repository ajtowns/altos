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

package org.altusmetrum.altoslib_4;

public class AltosTelemetrySatellite extends AltosTelemetryStandard {
	int		channels;
	AltosGPSSat[]	sats;

	public AltosTelemetrySatellite(int[] bytes) {
		super(bytes);

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

	public void update_state(AltosState state) {
		super.update_state(state);

		AltosGPS	gps = state.make_temp_gps(true);

		gps.cc_gps_sat = sats;
		state.set_temp_gps();
	}
}

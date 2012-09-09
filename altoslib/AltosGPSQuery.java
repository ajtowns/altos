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

package org.altusmetrum.AltosLib;

import java.util.concurrent.*;

class AltosGPSQuery extends AltosGPS {
	public AltosGPSQuery (AltosLink link, AltosConfigData config_data)
		throws TimeoutException, InterruptedException {
		boolean says_done = config_data.compare_version("1.0") >= 0;
		link.printf("g\n");
		for (;;) {
			String line = link.get_reply_no_dialog(5000);
			if (line == null)
				throw new TimeoutException();
			String[] bits = line.split("\\s+");
			if (bits.length == 0)
				continue;
			if (line.startsWith("Date:")) {
				if (bits.length < 2)
					continue;
				String[] d = bits[1].split(":");
				if (d.length < 3)
					continue;
				year = Integer.parseInt(d[0]) + 2000;
				month = Integer.parseInt(d[1]);
				day = Integer.parseInt(d[2]);
				continue;
			}
			if (line.startsWith("Time:")) {
				if (bits.length < 2)
					continue;
				String[] d = bits[1].split("/");
				if (d.length < 3)
					continue;
				hour = Integer.parseInt(d[0]);
				minute = Integer.parseInt(d[1]);
				second = Integer.parseInt(d[2]);
				continue;
			}
			if (line.startsWith("Lat/Lon:")) {
				if (bits.length < 3)
					continue;
				lat = Integer.parseInt(bits[1]) * 1.0e-7;
				lon = Integer.parseInt(bits[2]) * 1.0e-7;
				continue;
			}
			if (line.startsWith("Alt:")) {
				if (bits.length < 2)
					continue;
				alt = Integer.parseInt(bits[1]);
				continue;
			}
			if (line.startsWith("Flags:")) {
				if (bits.length < 2)
					continue;
				int status = Integer.decode(bits[1]);
				connected = (status & AltosLib.AO_GPS_RUNNING) != 0;
				locked = (status & AltosLib.AO_GPS_VALID) != 0;
				if (!says_done)
					break;
				continue;
			}
			if (line.startsWith("Sats:")) {
				if (bits.length < 2)
					continue;
				nsat = Integer.parseInt(bits[1]);
				cc_gps_sat = new AltosGPSSat[nsat];
				for (int i = 0; i < nsat; i++) {
					int	svid = Integer.parseInt(bits[2+i*2]);
					int	cc_n0 = Integer.parseInt(bits[3+i*2]);
					cc_gps_sat[i] = new AltosGPSSat(svid, cc_n0);
				}
			}
			if (line.startsWith("done"))
				break;
			if (line.startsWith("Syntax error"))
				break;
		}
	}
}


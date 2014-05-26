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

package org.altusmetrum.altoslib_4;

import java.util.concurrent.*;

public class AltosMag implements Cloneable {
	public double		x;
	public double		y;
	public double		z;

	public static double counts_per_gauss = 1090;

	public static double convert_gauss(int counts) {
		return (double) counts / counts_per_gauss;
	}

	public boolean parse_string(String line) {
//		if (line.startsWith("Syntax error")) {
//			x = y = z = 0;
//			return true;
//		}

		if (!line.startsWith("X:"))
			return false;

		String[] items = line.split("\\s+");

		if (items.length >= 6) {
			x = convert_gauss(Integer.parseInt(items[1]));
			y = convert_gauss(Integer.parseInt(items[3]));
			z = convert_gauss(Integer.parseInt(items[5]));
		}
		return true;
	}

	public AltosMag clone() {
		AltosMag n = new AltosMag();

		n.x = x;
		n.y = y;
		n.z = z;
		return n;
	}

	public AltosMag() {
		x = AltosLib.MISSING;
		y = AltosLib.MISSING;
		z = AltosLib.MISSING;
	}

	static public void update_state(AltosState state, AltosLink link, AltosConfigData config_data) throws InterruptedException {
		try {
			AltosMag	mag = new AltosMag(link);

			if (mag != null)
				state.set_mag(mag);
		} catch (TimeoutException te) {
		}
	}

	public AltosMag(AltosLink link) throws InterruptedException, TimeoutException {
		this();
		link.printf("M\n");
		for (;;) {
			String line = link.get_reply_no_dialog(5000);
			if (line == null) {
				throw new TimeoutException();
			}
			if (parse_string(line))
				break;
		}
	}
}

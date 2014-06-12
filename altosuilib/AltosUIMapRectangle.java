/*
 * Copyright © 2014 Keith Packard <keithp@keithp.com>
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

package org.altusmetrum.altosuilib_2;

public class AltosUIMapRectangle {
	AltosUILatLon	ul, lr;

	public AltosUIMapRectangle(AltosUILatLon a, AltosUILatLon b) {
		double	ul_lat, ul_lon;
		double	lr_lat, lr_lon;

		if (a.lat > b.lat) {
			ul_lat = a.lat;
			lr_lat = b.lat;
		} else {
			ul_lat = b.lat;
			lr_lat = a.lat;
		}
		if (a.lon < b.lon) {
			ul_lon = a.lon;
			lr_lon = b.lon;
		} else {
			ul_lon = b.lon;
			lr_lon = a.lon;
		}

		ul = new AltosUILatLon(ul_lat, ul_lon);
		lr = new AltosUILatLon(lr_lat, lr_lon);
	}
}

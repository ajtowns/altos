/*
 * Copyright © 2010 Keith Packard <keithp@keithp.com>
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

package org.altusmetrum.altoslib_2;

import java.lang.Math;

public class AltosGreatCircle implements Cloneable {
	public double	distance;
	public double	bearing;
	public double	range;
	public double	elevation;

	double sqr(double a) { return a * a; }

	static final double rad = Math.PI / 180;
	static final double earth_radius = 6371.2 * 1000;	/* in meters */

	public static final int BEARING_LONG = 0;
	public static final int BEARING_SHORT = 1;
	public static final int BEARING_VOICE = 2;

	public String bearing_words(int length) {
		String [][] bearing_string = {
			{
				"North", "North North East", "North East", "East North East",
				"East", "East South East", "South East", "South South East",
				"South", "South South West", "South West", "West South West",
				"West", "West North West", "North West", "North North West"
			}, {
				"N", "NNE", "NE", "ENE",
				"E", "ESE", "SE", "SSE",
				"S", "SSW", "SW", "WSW",
				"W", "WNW", "NW", "NNW"
			}, {
				"north", "nor nor east", "north east", "east nor east",
				"east", "east sow east", "south east", "sow sow east",
				"south", "sow sow west", "south west", "west sow west",
				"west", "west nor west", "north west", "nor nor west "
			}
		};
		return bearing_string[length][(int)((bearing / 90 * 8 + 1) / 2)%16];
	}

	public AltosGreatCircle (double start_lat, double start_lon, double start_alt,
				 double end_lat, double end_lon, double end_alt) {
		double lat1 = rad * start_lat;
		double lon1 = rad * -start_lon;
		double lat2 = rad * end_lat;
		double lon2 = rad * -end_lon;

		double d_lon = lon2 - lon1;

		/* From http://en.wikipedia.org/wiki/Great-circle_distance */
		double vdn = Math.sqrt(sqr(Math.cos(lat2) * Math.sin(d_lon)) +
				       sqr(Math.cos(lat1) * Math.sin(lat2) -
					   Math.sin(lat1) * Math.cos(lat2) * Math.cos(d_lon)));
		double vdd = Math.sin(lat1) * Math.sin(lat2) + Math.cos(lat1) * Math.cos(lat2) * Math.cos(d_lon);
		double d = Math.atan2(vdn,vdd);
		double course;

		if (Math.cos(lat1) < 1e-20) {
			if (lat1 > 0)
				course = Math.PI;
			else
				course = -Math.PI;
		} else {
			if (d < 1e-10)
				course = 0;
			else
				course = Math.acos((Math.sin(lat2)-Math.sin(lat1)*Math.cos(d)) /
						   (Math.sin(d)*Math.cos(lat1)));
			if (Math.sin(lon2-lon1) > 0)
				course = 2 * Math.PI-course;
		}
		distance = d * earth_radius;
		bearing = course * 180/Math.PI;

		double height_diff = end_alt - start_alt;
		range = Math.sqrt(distance * distance + height_diff * height_diff);
		elevation = Math.atan2(height_diff, distance) * 180 / Math.PI;
	}

	public AltosGreatCircle clone() {
		AltosGreatCircle n = new AltosGreatCircle();

		n.distance = distance;
		n.bearing = bearing;
		n.range = range;
		n.elevation = elevation;
		return n;
	}

	public AltosGreatCircle (double start_lat, double start_lon,
				 double end_lat, double end_lon) {
		this(start_lat, start_lon, 0, end_lat, end_lon, 0);
	}

	public AltosGreatCircle(AltosGPS start, AltosGPS end) {
		this(start.lat, start.lon, start.alt, end.lat, end.lon, end.alt);
	}

	public AltosGreatCircle() {
		distance = 0;
		bearing = 0;
		range = 0;
		elevation = 0;
	}
}

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

package altosui;

import java.lang.Math;

public class AltosGreatCircle {
	double	distance;
	double	bearing;

	double sqr(double a) { return a * a; }

	static final double rad = Math.PI / 180;
	static final double earth_radius = 6371.2 * 1000;	/* in meters */

	AltosGreatCircle (double start_lat, double start_lon,
			  double end_lat, double end_lon)
	{
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
	}
}

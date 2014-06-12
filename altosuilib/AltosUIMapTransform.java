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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.io.*;
import java.lang.Math;
import java.awt.geom.*;
import java.util.*;
import java.util.concurrent.*;
import org.altusmetrum.altoslib_4.*;

public class AltosUIMapTransform {

	double	scale_x, scale_y;

	double	offset_x, offset_y;

	public AltosUILatLon lat_lon (Point2D.Double point) {
		double lat, lon;
		double rads;

		lon = point.x/scale_x;
		rads = 2 * Math.atan(Math.exp(-point.y/scale_y));
		lat = Math.toDegrees(rads - Math.PI/2);

		return new AltosUILatLon(lat,lon);
	}

	public Point2D.Double screen_point(Point screen) {
		return new Point2D.Double(screen.x + offset_x, screen.y + offset_y);
	}

	public AltosUILatLon screen_lat_lon(Point screen) {
		return lat_lon(screen_point(screen));
	}

	public Point2D.Double point(AltosUILatLon lat_lon) {
		double x, y;
		double e;

		x = lat_lon.lon * scale_x;

		e = Math.sin(Math.toRadians(lat_lon.lat));
		e = Math.max(e,-(1-1.0E-15));
		e = Math.min(e,  1-1.0E-15 );

		y = 0.5*Math.log((1+e)/(1-e))*-scale_y;

		return new Point2D.Double(x, y);
	}

	public Point2D.Double screen(Point2D.Double point) {
		return new Point2D.Double(point.x - offset_x, point.y - offset_y);
	}

	public Point screen(Point point) {
		return new Point((int) (point.x - offset_x + 0.5),
				 (int) (point.y - offset_y + 0.5));
	}

	public Rectangle screen(AltosUIMapRectangle map_rect) {
		Point2D.Double	ul = screen(map_rect.ul);
		Point2D.Double	lr = screen(map_rect.lr);

		return new Rectangle((int) ul.x, (int) ul.y, (int) (lr.x - ul.x), (int) (lr.y - ul.y));
	}

	public Point2D.Double screen(AltosUILatLon lat_lon) {
		return screen(point(lat_lon));
	}

	private boolean has_location;

	public boolean has_location() {
		return has_location;
	}

	public AltosUIMapTransform(int width, int height, int zoom, AltosUILatLon centre_lat_lon) {
		scale_x = 256/360.0 * Math.pow(2, zoom);
		scale_y = 256/(2.0*Math.PI) * Math.pow(2, zoom);

		Point2D.Double centre_pt = point(centre_lat_lon);

		has_location = (centre_lat_lon.lat != 0 || centre_lat_lon.lon != 0);
		offset_x = centre_pt.x - width / 2.0;
		offset_y = centre_pt.y - height / 2.0;
	}
}

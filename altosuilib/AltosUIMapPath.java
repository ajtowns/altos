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

class PathPoint {
	AltosUILatLon	lat_lon;
	int		state;

	public PathPoint(AltosUILatLon lat_lon, int state) {
		this.lat_lon = lat_lon;
		this.state = state;
	}

	public boolean equals(PathPoint other) {
		if (other == null)
			return false;

		return lat_lon.equals(other.lat_lon) && state == other.state;
	}
}

public class AltosUIMapPath {

	LinkedList<PathPoint>	points = new LinkedList<PathPoint>();
	PathPoint		last_point = null;

	static public int stroke_width = 6;

	public void paint(Graphics2D g, AltosUIMapTransform t) {
		Point2D.Double	prev = null;

		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
				   RenderingHints.VALUE_ANTIALIAS_ON);
		g.setStroke(new BasicStroke(stroke_width, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND));

		for (PathPoint point : points) {
			Point2D.Double	cur = t.screen(point.lat_lon);
			if (prev != null) {
				Line2D.Double	line = new Line2D.Double (prev, cur);
				Rectangle	bounds = line.getBounds();

				if (g.hitClip(bounds.x, bounds.y, bounds.width, bounds.height)) {
					if (0 <= point.state && point.state < AltosUIMap.stateColors.length)
						g.setColor(AltosUIMap.stateColors[point.state]);
					else
						g.setColor(AltosUIMap.stateColors[AltosLib.ao_flight_invalid]);

					g.draw(line);
				}
			}
			prev = cur;
		}
	}

	public AltosUIMapRectangle add(double lat, double lon, int state) {
		PathPoint		point = new PathPoint(new AltosUILatLon (lat, lon), state);
		AltosUIMapRectangle	rect = null;

		if (!point.equals(last_point)) {
			if (last_point != null)
				rect = new AltosUIMapRectangle(last_point.lat_lon, point.lat_lon);
			points.add (point);
			last_point = point;
		}
		return rect;
	}

	public void clear () {
		points = new LinkedList<PathPoint>();
	}
}

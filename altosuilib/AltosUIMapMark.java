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

public class AltosUIMapMark {

	AltosUILatLon	lat_lon;
	int		state;

	static public int stroke_width = 6;

	public void paint(Graphics2D g, AltosUIMapTransform t) {

		Point2D.Double pt = t.screen(lat_lon);

		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
				   RenderingHints.VALUE_ANTIALIAS_ON);
		g.setStroke(new BasicStroke(stroke_width, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND));

		if (0 <= state && state < AltosUIMap.stateColors.length)
			g.setColor(AltosUIMap.stateColors[state]);
		else
			g.setColor(AltosUIMap.stateColors[AltosLib.ao_flight_invalid]);

		g.drawOval((int)pt.x-5, (int)pt.y-5, 10, 10);
		g.drawOval((int)pt.x-20, (int)pt.y-20, 40, 40);
		g.drawOval((int)pt.x-35, (int)pt.y-35, 70, 70);
	}

	public AltosUIMapMark (double lat, double lon, int state) {
		lat_lon = new AltosUILatLon(lat, lon);
		this.state = state;
	}
}

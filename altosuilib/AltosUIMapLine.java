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

public class AltosUIMapLine {
	AltosUILatLon	start, end;

	private Font	font = null;

	public void set_font(Font font) {
		this.font = font;
	}

	private AltosUILatLon lat_lon(MouseEvent e, AltosUIMapTransform t) {
		return t.screen_lat_lon(e.getPoint());
	}

	public void dragged(MouseEvent e, AltosUIMapTransform t) {
		end = lat_lon(e, t);
	}

	public void pressed(MouseEvent e, AltosUIMapTransform t) {
		start = lat_lon(e, t);
		end = null;
	}

	private String line_dist() {
		String	format;
		AltosGreatCircle	g = new AltosGreatCircle(start.lat, start.lon,
								 end.lat, end.lon);
		double	distance = g.distance;

		if (AltosConvert.imperial_units) {
			distance = AltosConvert.meters_to_feet(distance);
			if (distance < 10000) {
				format = "%4.0fft";
			} else {
				distance /= 5280;
				if (distance < 10)
					format = "%5.3fmi";
				else if (distance < 100)
					format = "%5.2fmi";
				else if (distance < 1000)
					format = "%5.1fmi";
				else
					format = "%5.0fmi";
			}
		} else {
			if (distance < 10000) {
				format = "%4.0fm";
			} else {
				distance /= 1000;
				if (distance < 100)
					format = "%5.2fkm";
				else if (distance < 1000)
					format = "%5.1fkm";
				else
					format = "%5.0fkm";
			}
		}
		return String.format(format, distance);
	}

	public void paint(Graphics2D g, AltosUIMapTransform t) {
		g.setColor(Color.BLUE);

		if (start == null || end == null)
			return;

		Line2D.Double line = new Line2D.Double(t.screen(start),
						       t.screen(end));

		g.draw(line);

		String	message = line_dist();
		g.setFont(font);
		g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
		Rectangle2D	bounds;
		bounds = font.getStringBounds(message, g.getFontRenderContext());

		float x = (float) line.x1;
		float y = (float) line.y1 + (float) bounds.getHeight() / 2.0f;

		if (line.x1 < line.x2) {
			x -= (float) bounds.getWidth() + 2.0f;
		} else {
			x += 2.0f;
		}
		g.drawString(message, x, y);
	}
}

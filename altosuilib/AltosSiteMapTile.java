/*
 * Copyright © 2010 Anthony Towns <aj@erisian.com.au>
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
import java.awt.image.*;
import javax.swing.*;
import javax.imageio.*;
import java.awt.geom.*;
import java.io.*;
import java.util.*;
import java.awt.RenderingHints.*;
import org.altusmetrum.altoslib_4.*;

class AltosPoint {
	Point2D.Double	pt;
	int		state;

	AltosPoint(Point2D.Double pt, int state) {
		this.pt = pt;
		this.state = state;
	}
}

public class AltosSiteMapTile extends JComponent {
	int px_size;
	File file;
	int status;

	Point2D.Double	boost;
	Point2D.Double	landed;

	LinkedList<AltosPoint>	points;

	public synchronized void queue_repaint() {
		SwingUtilities.invokeLater(new Runnable() {
				public void run() {
					repaint();
				}
			});
	}

	public void load_map(File pngFile) {
		file = pngFile;
		queue_repaint();
	}

	private Font	font = null;

	public void set_font(Font font) {
		this.font = font;
		this.status = AltosSiteMapCache.success;
		queue_repaint();
	}

	public void set_status(int status) {
		file = null;
		this.status = status;
		queue_repaint();
	}

	public void clearMap() {
		boost = null;
		landed = null;
		points = null;
		file = null;
		status = AltosSiteMapCache.success;
		queue_repaint();
	}

	static Color stateColors[] = {
		Color.WHITE,  // startup
		Color.WHITE,  // idle
		Color.WHITE,  // pad
		Color.RED,    // boost
		Color.PINK,   // fast
		Color.YELLOW, // coast
		Color.CYAN,   // drogue
		Color.BLUE,   // main
		Color.BLACK   // landed
	};

	private void draw_circle(Graphics g, Point2D.Double pt) {
		g.drawOval((int)pt.x-5, (int)pt.y-5, 10, 10);
		g.drawOval((int)pt.x-20, (int)pt.y-20, 40, 40);
		g.drawOval((int)pt.x-35, (int)pt.y-35, 70, 70);
	}

	public void set_boost(Point2D.Double boost) {
		this.boost = boost;
		queue_repaint();
	}

	public void paint(Graphics g) {
		Graphics2D	g2d = (Graphics2D) g;
		AltosPoint	prev = null;
		Image		img = null;

		if (file != null)
			img = AltosSiteMapCache.get_image(this, file, px_size, px_size);

		if (img != null) {
			g2d.drawImage(img, 0, 0, null);
		} else {
			g2d.setColor(Color.GRAY);
			g2d.fillRect(0, 0, getWidth(), getHeight());
			String	message = null;
			switch (status) {
			case AltosSiteMapCache.loading:
				message = "Loading...";
				break;
			case AltosSiteMapCache.bad_request:
				message = "Internal error";
				break;
			case AltosSiteMapCache.failed:
				message = "Network error, check connection";
				break;
			case AltosSiteMapCache.forbidden:
				message = "Too many requests, try later";
				break;
			}
			if (message != null && font != null) {
				g2d.setFont(font);
				g2d.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
				Rectangle2D	bounds;
				bounds = font.getStringBounds(message, g2d.getFontRenderContext());

				float x = getWidth() / 2.0f;
				float y = getHeight() / 2.0f;
				x = x - (float) bounds.getWidth() / 2.0f;
				y = y - (float) bounds.getHeight() / 2.0f;
				g2d.setColor(Color.BLACK);
				g2d.drawString(message, x, y);
			}
		}

		g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
				   RenderingHints.VALUE_ANTIALIAS_ON);
		g2d.setStroke(new BasicStroke(6, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND));

		if (points != null) {
			for (AltosPoint point : points) {
				if (prev != null) {
					if (0 <= point.state && point.state < stateColors.length)
						g2d.setColor(stateColors[point.state]);
					g2d.draw(new Line2D.Double(prev.pt, point.pt));
				}
				prev = point;
			}
		}
		if (boost != null) {
			g2d.setColor(Color.RED);
			draw_circle(g2d, boost);
		}
		if (landed != null) {
			g2d.setColor(Color.BLACK);
			draw_circle(g2d, landed);
		}
	}

	public synchronized void show(int state, Point2D.Double last_pt, Point2D.Double pt)
	{
		if (points == null)
			points = new LinkedList<AltosPoint>();

		points.add(new AltosPoint(pt, state));

		if (state == AltosLib.ao_flight_boost && boost == null)
			boost = pt;
		if (state == AltosLib.ao_flight_landed && landed == null)
			landed = pt;
		queue_repaint();
	}

	public AltosSiteMapTile(int in_px_size) {
		px_size = in_px_size;
		setPreferredSize(new Dimension(px_size, px_size));
	}
}

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

package altosui;

import java.awt.*;
import java.awt.image.*;
import java.awt.event.*;
import javax.swing.*;
import javax.imageio.ImageIO;
import javax.swing.table.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.lang.Math;
import java.awt.geom.Point2D;
import java.awt.geom.Line2D;
import org.altusmetrum.AltosLib.*;

public class AltosSiteMapTile extends JLayeredPane {
	JLabel mapLabel;
	JLabel draw;
	Graphics2D g2d;
	int px_size;

	public void loadMap(ImageIcon icn) {
		mapLabel.setIcon(icn);
	}

	public void clearMap() {
		fillLabel(mapLabel, Color.GRAY, px_size);
		g2d = fillLabel(draw, new Color(127,127,127,0), px_size);
		g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
				     RenderingHints.VALUE_ANTIALIAS_ON);
		g2d.setStroke(new BasicStroke(6, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND));
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

	private boolean drawn_landed_circle = false;
	private boolean drawn_boost_circle = false;
	public synchronized void show(AltosState state, int crc_errors,
				      Point2D.Double last_pt, Point2D.Double pt)
	{
		if (0 <= state.state && state.state < stateColors.length) {
			g2d.setColor(stateColors[state.state]);
		}
		g2d.draw(new Line2D.Double(last_pt, pt));

		if (state.state == 3 && !drawn_boost_circle) {
			drawn_boost_circle = true;
			g2d.setColor(Color.RED);
			g2d.drawOval((int)last_pt.x-5, (int)last_pt.y-5, 10, 10);
			g2d.drawOval((int)last_pt.x-20, (int)last_pt.y-20, 40, 40);
			g2d.drawOval((int)last_pt.x-35, (int)last_pt.y-35, 70, 70);
		}
		if (state.state == 8 && !drawn_landed_circle) {
			drawn_landed_circle = true;
			g2d.setColor(Color.BLACK);
			g2d.drawOval((int)pt.x-5, (int)pt.y-5, 10, 10);
			g2d.drawOval((int)pt.x-20, (int)pt.y-20, 40, 40);
			g2d.drawOval((int)pt.x-35, (int)pt.y-35, 70, 70);
		}

		repaint();
	}

	public void draw_circle(Point2D.Double pt) {
		g2d.setColor(Color.RED);
		g2d.drawOval((int)pt.x-5, (int)pt.y-5, 10, 10);
		g2d.drawOval((int)pt.x-20, (int)pt.y-20, 40, 40);
		g2d.drawOval((int)pt.x-35, (int)pt.y-35, 70, 70);
	}

	public static Graphics2D fillLabel(JLabel l, Color c, int px_size) {
		BufferedImage img = new BufferedImage(px_size, px_size,
						      BufferedImage.TYPE_INT_ARGB);
		Graphics2D g = img.createGraphics();
		g.setColor(c);
		g.fillRect(0, 0, px_size, px_size);
		l.setIcon(new ImageIcon(img));
		return g;
	}

	public AltosSiteMapTile(int in_px_size) {
		px_size = in_px_size;
		setPreferredSize(new Dimension(px_size, px_size));

		mapLabel = new JLabel();
		fillLabel(mapLabel, Color.GRAY, px_size);
		mapLabel.setOpaque(true);
		mapLabel.setBounds(0, 0, px_size, px_size);
		add(mapLabel, new Integer(0));

		draw = new JLabel();
		g2d = fillLabel(draw, new Color(127, 127, 127, 0), px_size);
		g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
				     RenderingHints.VALUE_ANTIALIAS_ON);
		g2d.setStroke(new BasicStroke(6, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND));
		draw.setBounds(0, 0, px_size, px_size);
		draw.setOpaque(false);

		add(draw, new Integer(1));
	}
}

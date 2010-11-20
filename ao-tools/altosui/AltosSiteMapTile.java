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

public class AltosSiteMapTile extends JLayeredPane {
    Point2D.Double coord_pt;
    Point2D.Double last_pt;

    JLabel mapLabel;
    JLabel draw;
    Graphics2D g2d;

    public void loadMap(ImageIcon icn) {
        mapLabel.setIcon(icn);
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

    boolean drawn_landed_circle = false;
    boolean drawn_boost_circle = false;
    public void show(AltosState state, int crc_errors, Point2D.Double pt) {
        if (last_pt == null) {
            // setLocation(state.pad_lat, state.pad_lon);
            // loadMap();
            last_pt = pt;
        }

        if (pt != last_pt) {
            if (0 <= state.state && state.state < stateColors.length) {
                g2d.setColor(stateColors[state.state]);
            }
            g2d.draw(new Line2D.Double(last_pt, pt));
        }

        int px_size = getWidth();
        if (0 <= pt.x && pt.x < px_size) {
            if (0 <= pt.y && pt.y < px_size) {
                int dx = 500, dy = 250;
                if (state.state > 2) {
                    dx = Math.min(200, 20 + (int) Math.abs(last_pt.x - pt.x));
                    dy = Math.min(100, 10 + (int) Math.abs(last_pt.y - pt.y));
                }
                Rectangle r = new Rectangle((int)pt.x-dx, (int)pt.y-dy, 
                                            dx*2, dy*2);
                scrollRectToVisible(r);
            }
        }

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

        last_pt = pt;
        repaint();
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

    public AltosSiteMapTile(int px_size) {
        setPreferredSize(new Dimension(px_size, px_size));

        mapLabel = new JLabel();
        fillLabel(mapLabel, Color.GRAY, px_size);
        mapLabel.setOpaque(true);
        mapLabel.setBounds(0, 0, px_size, px_size);
        add(mapLabel, new Integer(0));

        draw = new JLabel();
        g2d = fillLabel(draw, new Color(127, 127, 127, 0), px_size);
        draw.setBounds(0, 0, px_size, px_size);
        draw.setOpaque(false);

        add(draw, new Integer(1));
    }
}


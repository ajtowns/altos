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
import javax.swing.event.MouseInputAdapter;
import javax.imageio.ImageIO;
import javax.swing.table.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.lang.Math;
import java.awt.geom.Point2D;
import java.awt.geom.Line2D;

public class AltosSiteMap extends JScrollPane implements AltosFlightDisplay {
    public void reset() {
        // nothing
    }
    public void show(AltosState state, int crc_errors) {
        for (int x = 0; x < mapTiles.length; x++) {
            mapTiles[x].show(state, crc_errors);
        }
    }
  
    AltosSiteMapTile [] mapTiles = new AltosSiteMapTile[9];

    class GrabNDrag extends MouseInputAdapter {
        private JComponent scroll;
        private Point startPt = new Point();

        public GrabNDrag(JComponent parent) {
            scroll = parent;
        }

        public void mousePressed(MouseEvent e) {
            startPt.setLocation(e.getPoint());
        }
        public void mouseDragged(MouseEvent e) {
            int xd = e.getX() - startPt.x;
            int yd = e.getY() - startPt.y;

            Rectangle r = scroll.getVisibleRect();
            r.x -= xd;
            r.y -= yd;
            scroll.scrollRectToVisible(r);
        }
    }

    public AltosSiteMap() {
        JComponent comp = new JComponent() {
            GrabNDrag scroller = new GrabNDrag(this);
            {
                addMouseMotionListener(scroller);
                addMouseListener(scroller);
                setAutoscrolls(true);
            }
        };

        GridBagLayout layout = new GridBagLayout();
        comp.setLayout(layout);

        GridBagConstraints c = new GridBagConstraints();
        c.anchor = GridBagConstraints.CENTER;
        c.fill = GridBagConstraints.BOTH;

        for (int x = 0; x < 9; x++) {
            c.gridx = x % 3; c.gridy = x / 3;
            mapTiles[x] = new AltosSiteMapTile((x%3)-1, (x/3)-1);
            layout.setConstraints(mapTiles[x], c);
            comp.add(mapTiles[x]);
        }
        setViewportView(comp);
    }
}


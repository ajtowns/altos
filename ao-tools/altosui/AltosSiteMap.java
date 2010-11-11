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

public class AltosSiteMap extends JComponent implements AltosFlightDisplay {
    double lat, lng;
    int zoom;
    double scale_x, scale_y;
    Point2D.Double coord_pt;
    Point2D.Double last_pt;

    Graphics2D g2d;

    private void setLocation(double new_lat, double new_lng, int new_zoom) {
        lat = new_lat;
        lng = new_lng;
        zoom = new_zoom;
        scale_x = 256/360.0 * Math.pow(2, zoom);
        scale_y = 256/(2.0*Math.PI) * Math.pow(2, zoom);
        coord_pt = pt(lat, lng, new Point2D.Double(0,0));
        coord_pt.x = 320-coord_pt.x;
        coord_pt.y = 320-coord_pt.y;
        last_pt = null;
    }

    private static double limit(double v, double lo, double hi) {
        if (v < lo)
            return lo;
        if (hi < v)
            return hi;
        return v;
    }

    // based on google js
    //  http://maps.gstatic.com/intl/en_us/mapfiles/api-3/2/10/main.js
    // search for fromLatLngToPoint and fromPointToLatLng
    private Point2D.Double pt(double lat, double lng) {
        return pt(lat, lng, coord_pt);
    }

    private Point2D.Double pt(double lat, double lng, Point2D.Double centre) {
        Point2D.Double res = new Point2D.Double();
        double e;

        res.x = centre.x + lng*scale_x;
        e = limit(Math.sin(Math.toRadians(lat)),-(1-1.0E-15),1-1.0E-15);
        res.y = centre.y + 0.5*Math.log((1+e)/(1-e))*-scale_y;
        return res;
    }


    public void reset() {
        // ?
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

    public void show(AltosState state, int crc_errors) {
        if (!state.gps_ready)
            return;
        Point2D.Double pt = pt(state.gps.lat, state.gps.lon);
        if (last_pt != null && pt != last_pt) {
            if (0 <= state.state && state.state < stateColors.length) {
                g2d.setColor(stateColors[state.state]);
            }
            g2d.draw(new Line2D.Double(last_pt, pt));
        }
        last_pt = pt;
        repaint();
    }
    
    public AltosSiteMap() {
        GridBagLayout layout = new GridBagLayout();
        setLayout(layout);

        GridBagConstraints c = new GridBagConstraints();

        setLocation(-27.850, 152.960, 15);
        String pngfile = "/home/aj/qrs-S27.850,W152.960-15.png";

        c.gridx = 0; c.gridy = 0;
        c.weightx = 1; c.weighty = 1;
        c.anchor = GridBagConstraints.CENTER;
        c.fill = GridBagConstraints.BOTH;

        try {
            BufferedImage myPicture = ImageIO.read(new File(pngfile));
            g2d = myPicture.createGraphics();
            JLabel picLabel = new JLabel(new ImageIcon( myPicture ));
            JScrollPane scrollPane = new JScrollPane(picLabel);
            layout.setConstraints(scrollPane, c);
            add(scrollPane);
        } catch (Exception e) { 
            throw new RuntimeException(e);
        };
    }
}


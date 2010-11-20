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
    int zoom;
    double scale_x, scale_y;
    Point2D.Double coord_pt;
    Point2D.Double last_pt;

    JLabel mapLabel;
    JLabel draw;
    Graphics2D g2d;

    int off_x;
    int off_y;

    static final int px_size = 512;

    private void loadMap() {
        Point2D.Double map_latlng = latlng(px_size/2, px_size/2);
        File pngfile = new File(AltosPreferences.logdir(), 
                                FileCoord(map_latlng, zoom));
        try {
            mapLabel.setIcon(new ImageIcon(ImageIO.read(pngfile)));
        } catch (Exception e) { 
            // throw new RuntimeException(e);
            System.out.printf("# Failed to find file %s\n", pngfile);
            System.out.printf(" wget -O '%s' 'http://maps.google.com/maps/api/staticmap?center=%.6f,%.6f&zoom=%d&size=%dx%d&sensor=false&maptype=hybrid&format=png32'\n", pngfile, map_latlng.x, map_latlng.y, zoom, px_size, px_size);
        }
    }

    private boolean setLocation(double lat, double lng) {
        Point2D.Double north_step;
        double step_nm = 0.5;
        for (zoom = 3; zoom < 22; zoom++) {
            coord_pt = pt(lat, lng, new Point2D.Double(0,0), zoom);
            north_step = pt(lat+step_nm/60.0, lng, 
                    new Point2D.Double(0,0), zoom);
            if (coord_pt.y - north_step.y > px_size/2)
                break;
        }
        coord_pt.x = -px_size * Math.floor(coord_pt.x/px_size + off_x);
        coord_pt.y = -px_size * Math.floor(coord_pt.y/px_size + off_y);

        scale_x = 256/360.0 * Math.pow(2, zoom);
        scale_y = 256/(2.0*Math.PI) * Math.pow(2, zoom);

        last_pt = null;

        return true;
    }

    private static double limit(double v, double lo, double hi) {
        if (v < lo)
            return lo;
        if (hi < v)
            return hi;
        return v;
    }

    private static String FileCoord(Point2D.Double latlng, int zoom) {
        double lat, lng;
        lat = latlng.x;
        lng = latlng.y;
        return FileCoord(lat, lng, zoom);
    }
    private static String FileCoord(double lat, double lng, int zoom) {
        char chlat = lat < 0 ? 'S' : 'N';
        char chlng = lng < 0 ? 'E' : 'W';
        if (lat < 0) lat = -lat;
        if (lng < 0) lng = -lng;
        return String.format("map-%c%.6f,%c%.6f-%d.png",
                chlat, lat, chlng, lng, zoom);
    }


    // based on google js
    //  http://maps.gstatic.com/intl/en_us/mapfiles/api-3/2/10/main.js
    // search for fromLatLngToPoint and fromPointToLatLng
    private Point2D.Double pt(double lat, double lng) {
        return pt(lat, lng, coord_pt, scale_x, scale_y);
    }

    private static Point2D.Double pt(double lat, double lng, 
            Point2D.Double centre, int zoom)
    {
        double scale_x = 256/360.0 * Math.pow(2, zoom);
        double scale_y = 256/(2.0*Math.PI) * Math.pow(2, zoom);
        return pt(lat, lng, centre, scale_x, scale_y);
    }

    private static Point2D.Double pt(double lat, double lng, 
            Point2D.Double centre, double scale_x, double scale_y)
    {
        Point2D.Double res = new Point2D.Double();
        double e;

        res.x = centre.x + lng*scale_x;
        e = limit(Math.sin(Math.toRadians(lat)),-(1-1.0E-15),1-1.0E-15);
        res.y = centre.y + 0.5*Math.log((1+e)/(1-e))*-scale_y;
        return res;
    }

    private Point2D.Double latlng(double x, double y) {
        return latlng(new Point2D.Double(x,y), coord_pt);
    }
    private Point2D.Double latlng(Point2D.Double pt) {
        return latlng(pt, coord_pt);
    }
    private Point2D.Double latlng(Point2D.Double pt, Point2D.Double centre) {
        double lat, lng;
        double rads;

        lng = (pt.x - centre.x)/scale_x;
        rads = 2 * Math.atan(Math.exp((pt.y-centre.y)/-scale_y));
        lat = Math.toDegrees(rads - Math.PI/2);
                                                                    
        return new Point2D.Double(lat,lng);
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
    public void show(AltosState state, int crc_errors) {
        if (!state.gps_ready) {
            if (state.pad_lat == 0 && state.pad_lon == 0)
                return;
            if (state.ngps < 3)
                return;
        }

        if (last_pt == null) {
            setLocation(state.pad_lat, state.pad_lon);
            loadMap();
            last_pt = pt;
        }

        Point2D.Double pt = pt(state.gps.lat, state.gps.lon);
        if (pt != last_pt) {
            if (0 <= state.state && state.state < stateColors.length) {
                g2d.setColor(stateColors[state.state]);
            }
            g2d.draw(new Line2D.Double(last_pt, pt));
        }

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

        if (state.state == 8 && !drawn_landed_circle) {
            drawn_landed_circle = true;
            g2d.setColor(Color.RED);
            g2d.drawOval((int)pt.x-5, (int)pt.y-5, 10, 10);
            g2d.drawOval((int)pt.x-20, (int)pt.y-20, 40, 40);
            g2d.drawOval((int)pt.x-35, (int)pt.y-35, 70, 70);
        }

        last_pt = pt;
        repaint();
    }

    public static Graphics2D fillLabel(JLabel l, Color c) {
        BufferedImage img = new BufferedImage(px_size, px_size,
                BufferedImage.TYPE_INT_ARGB);
        Graphics2D g = img.createGraphics();
        g.setColor(c);
        g.fillRect(0, 0, px_size, px_size);
        l.setIcon(new ImageIcon(img));
        return g;
    }

    public AltosSiteMapTile(int x_tile_offset, int y_tile_offset) {
        setPreferredSize(new Dimension(px_size, px_size));

        mapLabel = new JLabel();
        fillLabel(mapLabel, Color.GRAY);
        mapLabel.setOpaque(true);
        mapLabel.setBounds(0, 0, px_size, px_size);
        add(mapLabel, new Integer(0));

        draw = new JLabel();
        g2d = fillLabel(draw, new Color(127, 127, 127, 0));
        draw.setBounds(0, 0, px_size, px_size);
        draw.setOpaque(false);

        add(draw, new Integer(1));

        off_x = x_tile_offset;
        off_y = y_tile_offset;
    }
}


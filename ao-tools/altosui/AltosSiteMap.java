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
	// preferred vertical step in a tile in naut. miles
	// will actually choose a step size between x and 2x, where this
	// is 1.5x
	static final double tile_size_nmi = 1.5;

	static final int px_size = 512;

	private static Point2D.Double translatePoint(Point2D.Double p,
			Point2D.Double d)
	{
		return new Point2D.Double(p.x + d.x, p.y + d.y);
	}

	static class LatLng {
		public double lat, lng;
		public LatLng(double lat, double lng) {
			this.lat = lat;
			this.lng = lng;
		}
	}

	// based on google js
	//  http://maps.gstatic.com/intl/en_us/mapfiles/api-3/2/10/main.js
	// search for fromLatLngToPoint and fromPointToLatLng
	private static Point2D.Double pt(LatLng latlng, int zoom) {
		double scale_x = 256/360.0 * Math.pow(2, zoom);
		double scale_y = 256/(2.0*Math.PI) * Math.pow(2, zoom);
		return pt(latlng, scale_x, scale_y);
	}

	private static Point2D.Double pt(LatLng latlng,
					 double scale_x, double scale_y)
	{
		Point2D.Double res = new Point2D.Double();
		double e;

		res.x = latlng.lng * scale_x;

		e = Math.sin(Math.toRadians(latlng.lat));
		e = Math.max(e,-(1-1.0E-15));
		e = Math.min(e,  1-1.0E-15 );

		res.y = 0.5*Math.log((1+e)/(1-e))*-scale_y;
		return res;
	}

	static private LatLng latlng(Point2D.Double pt,
				     double scale_x, double scale_y)
	{
		double lat, lng;
		double rads;

		lng = pt.x/scale_x;
		rads = 2 * Math.atan(Math.exp(-pt.y/scale_y));
		lat = Math.toDegrees(rads - Math.PI/2);

		return new LatLng(lat,lng);
	}

	int zoom;
	double scale_x, scale_y;

	private Point2D.Double pt(double lat, double lng) {
		return pt(new LatLng(lat, lng), scale_x, scale_y);
	}

	private LatLng latlng(double x, double y) {
		return latlng(new Point2D.Double(x,y), scale_x, scale_y);
	}
	private LatLng latlng(Point2D.Double pt) {
		return latlng(pt, scale_x, scale_y);
	}

	AltosSiteMapTile [] mapTiles = new AltosSiteMapTile[9];
	Point2D.Double [] tileOffset = new Point2D.Double[9];

	private Point2D.Double getBaseLocation(double lat, double lng) {
		Point2D.Double locn, north_step;

		zoom = 2;
		// stupid loop structure to please Java's control flow analysis
		do {
			zoom++;
			scale_x = 256/360.0 * Math.pow(2, zoom);
			scale_y = 256/(2.0*Math.PI) * Math.pow(2, zoom);
			locn = pt(lat, lng);
			north_step = pt(lat+tile_size_nmi*4/3/60.0, lng);
			if (locn.y - north_step.y > px_size)
				break;
		} while (zoom < 22);
		locn.x = -px_size * Math.floor(locn.x/px_size);
		locn.y = -px_size * Math.floor(locn.y/px_size);
		return locn;
	}

	public void reset() {
		// nothing
	}

	private void bgLoadMap(final int i,
			       final File pngfile, final String pngurl)
	{
		Thread thread = new Thread() {
			public void run() {
				ImageIcon res;
				res = AltosSiteMapCache.fetchAndLoadMap(pngfile, pngurl);
				if (res != null) {
					mapTiles[i].loadMap(res);
				} else {
					System.out.printf("# Failed to fetch file %s\n", pngfile);
					System.out.printf(" wget -O '%s' ''\n", pngfile, pngurl);
				}
			}
		};
		thread.start();
	}

	public static void prefetchMaps(double lat, double lng, int w, int h) {
		AltosPreferences.init(null);

		AltosSiteMap asm = new AltosSiteMap(true);
		Point2D.Double c = asm.getBaseLocation(lat, lng);
		Point2D.Double p = new Point2D.Double();
		Point2D.Double p2;
		int dx = -w/2, dy = -h/2;
		for (int y = dy; y < h+dy; y++) {
			for (int x = dx; x < w+dx; x++) {
				LatLng map_latlng = asm.latlng(
							    -c.x + x*px_size + px_size/2,
							    -c.y + y*px_size + px_size/2);
				File pngfile = asm.MapFile(map_latlng.lat, map_latlng.lng);
				String pngurl = asm.MapURL(map_latlng.lat, map_latlng.lng);
				if (pngfile.exists()) {
					System.out.printf("Already have %s\n", pngfile);
				} else if (AltosSiteMapCache.fetchMap(pngfile, pngurl)) {
					System.out.printf("Fetched map %s\n", pngfile);
				} else {
					System.out.printf("# Failed to fetch file %s\n", pngfile);
					System.out.printf(" wget -O '%s' ''\n", pngfile, pngurl);
				}
			}
		}
	}

	private void initMaps(double lat, double lng) {
		Point2D.Double c = getBaseLocation(lat, lng);
		Point2D.Double p = new Point2D.Double();

		for (int i = 0; i < 9; i++) {
			int x = i%3 - 1, y = i/3 - 1;

			tileOffset[i] = new Point2D.Double(
				c.x - x*px_size, p.y = c.y - y*px_size);
			LatLng map_latlng = latlng(
						    -tileOffset[i].x+px_size/2,
						    -tileOffset[i].y+px_size/2);

			File pngfile = MapFile(map_latlng.lat, map_latlng.lng);
			String pngurl = MapURL(map_latlng.lat, map_latlng.lng);
			bgLoadMap(i, pngfile, pngurl);
		}
	}

	private File MapFile(double lat, double lng) {
		char chlat = lat < 0 ? 'S' : 'N';
		char chlng = lng < 0 ? 'E' : 'W';
		if (lat < 0) lat = -lat;
		if (lng < 0) lng = -lng;
		return new File(AltosPreferences.logdir(),
				String.format("map-%c%.6f,%c%.6f-%d.png",
					      chlat, lat, chlng, lng, zoom));
	}

	private String MapURL(double lat, double lng) {
		return String.format("http://maps.google.com/maps/api/staticmap?center=%.6f,%.6f&zoom=%d&size=%dx%d&sensor=false&maptype=hybrid&format=png32", lat, lng, zoom, px_size, px_size);
	}

	boolean initialised = false;
	public void show(AltosState state, int crc_errors) {
		// if insufficient gps data, nothing to update
		if (state.gps == null)
			return;
		if (!state.gps.locked) {
			if (state.pad_lat == 0 && state.pad_lon == 0)
				return;
			if (state.gps.nsat < 4)
				return;
		}

		if (!initialised) {
			initMaps(state.pad_lat, state.pad_lon);
			initialised = true;
		}

		Point2D.Double pt = pt(state.gps.lat, state.gps.lon);
		for (int x = 0; x < mapTiles.length; x++) {
			mapTiles[x].show(state, crc_errors,
					 translatePoint(pt, tileOffset[x]));
		}
	}

	private AltosSiteMap(boolean knowWhatYouAreDoing) {
		if (!knowWhatYouAreDoing) {
			throw new RuntimeException("Arggh.");
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

		// put some space between the map tiles, debugging only
		// c.insets = new Insets(5, 5, 5, 5);
		for (int x = 0; x < 9; x++) {
			c.gridx = x % 3;
			c.gridy = x / 3;
			mapTiles[x] = new AltosSiteMapTile(px_size);
			layout.setConstraints(mapTiles[x], c);
			comp.add(mapTiles[x]);
		}
		setViewportView(comp);
		setPreferredSize(new Dimension(500,200));
	}
}

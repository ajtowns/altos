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
	static final double tile_size_nmi = 0.75;

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

	Vector<AltosSiteMapTile> mapTiles = new Vector<AltosSiteMapTile>();
	Point2D.Double centre;

	private Point tileOffset(AltosSiteMapTile tile) {
		GridBagConstraints c = layout.getConstraints(tile);
		return new Point(c.gridx - 100, c.gridy - 100);
	}
	private Point2D.Double tileCoordOffset(AltosSiteMapTile tile) {
		Point p = tileOffset(tile);
		return new Point2D.Double(centre.x - p.x*px_size,
					  centre.y - p.y * px_size);
	}

	private Point tileOffset(Point2D.Double p) {
		return new Point((int)Math.floor((centre.x+p.x)/px_size),
				 (int)Math.floor((centre.y+p.y)/px_size));
	}

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

	private void bgLoadMap(final AltosSiteMapTile tile,
			       final File pngfile, final String pngurl)
	{
		//System.out.printf("Loading/fetching map %s\n", pngfile);
		Thread thread = new Thread() {
			public void run() {
				ImageIcon res;
				res = AltosSiteMapCache.fetchAndLoadMap(pngfile, pngurl);
				if (res != null) {
					tile.loadMap(res);
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
		asm.centre = asm.getBaseLocation(lat, lng);

		Point2D.Double p = new Point2D.Double();
		Point2D.Double p2;
		int dx = -w/2, dy = -h/2;
		for (int y = dy; y < h+dy; y++) {
			for (int x = dx; x < w+dx; x++) {
				LatLng map_latlng = asm.latlng(
							    -asm.centre.x + x*px_size + px_size/2,
							    -asm.centre.y + y*px_size + px_size/2);
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

	private void initMap(AltosSiteMapTile tile) {
		Point2D.Double offset = tileCoordOffset(tile);

		LatLng map_latlng = latlng(px_size/2-offset.x, px_size/2-offset.y);

		File pngfile = MapFile(map_latlng.lat, map_latlng.lng);
		String pngurl = MapURL(map_latlng.lat, map_latlng.lng);
		bgLoadMap(tile, pngfile, pngurl);
	}

	private void initMaps(double lat, double lng) {
		centre = getBaseLocation(lat, lng);

		for (AltosSiteMapTile tile : mapTiles) {
			initMap(tile);
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
	Point2D.Double last_pt = null;
	int last_state = -1;
	public void show(final AltosState state, final int crc_errors) {
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

		final Point2D.Double pt = pt(state.gps.lat, state.gps.lon);
		if (last_pt == pt && last_state == state.state)
			return;

		if (last_pt == null) {
			last_pt = pt;
		}
		boolean in_any = false;
		for (AltosSiteMapTile tile : mapTiles) {
			Point2D.Double ref, lref;
			ref = translatePoint(pt, tileCoordOffset(tile));
			lref = translatePoint(last_pt, tileCoordOffset(tile));
			tile.show(state, crc_errors, lref, ref);
			if (0 <= ref.x && ref.x < px_size)
				if (0 <= ref.y && ref.y < px_size)
					in_any = true;
		}
		if (!in_any) {
			try {
				SwingUtilities.invokeAndWait( new Runnable() {
					public void run() {
						AltosSiteMapTile tile = addTileAt(tileOffset(pt));
						setViewportView(comp);

						Point2D.Double ref, lref;
						ref = translatePoint(pt, tileCoordOffset(tile));
						lref = translatePoint(last_pt, tileCoordOffset(tile));
						tile.show(state, crc_errors, lref, ref);

						initMap(tile);
					}
				} );
			} catch (Exception e) {
				// pray
			}
		}
		last_pt = pt;
		last_state = state.state;
	}

	private AltosSiteMapTile addTileAt(Point offset) {
		GridBagConstraints c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.BOTH;

		// put some space between the map tiles, debugging only
		// c.insets = new Insets(5, 5, 5, 5);
		//
		AltosSiteMapTile t = new AltosSiteMapTile(px_size);
		mapTiles.add(t);
		c.gridx = offset.x + 100;
		c.gridy = offset.y + 100;
		layout.setConstraints(t, c);
		comp.add(t);

		return t;
	}

	private AltosSiteMap(boolean knowWhatYouAreDoing) {
		if (!knowWhatYouAreDoing) {
			throw new RuntimeException("Arggh.");
		}
	}

	JComponent comp;
	private GridBagLayout layout;

	public AltosSiteMap() {
		comp = new JComponent() {
			GrabNDrag scroller = new GrabNDrag(this);
			{
				addMouseMotionListener(scroller);
				addMouseListener(scroller);
				setAutoscrolls(true);
			}
		};

		layout = new GridBagLayout();
		comp.setLayout(layout);

		for (int x = -1; x <= 1; x++) {
			for (int y = -1; y <= 1; y++) {
				addTileAt(new Point(x, y));
			}
		}
		setViewportView(comp);
		setPreferredSize(new Dimension(500,200));
	}
}

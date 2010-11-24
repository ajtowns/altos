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

	static final int MAX_TILE_DELTA = 100;

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

	HashMap<Point,AltosSiteMapTile> mapTiles = new HashMap<Point,AltosSiteMapTile>();
	Point2D.Double centre;

	private Point2D.Double tileCoordOffset(Point p) {
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

	private void initMap(AltosSiteMapTile tile, Point offset) {
		Point2D.Double coord = tileCoordOffset(offset);

		LatLng map_latlng = latlng(px_size/2-coord.x, px_size/2-coord.y);

		File pngfile = MapFile(map_latlng.lat, map_latlng.lng);
		String pngurl = MapURL(map_latlng.lat, map_latlng.lng);
		bgLoadMap(tile, pngfile, pngurl);
	}

	private void initMaps(double lat, double lng) {
		centre = getBaseLocation(lat, lng);

		for (Point k : mapTiles.keySet()) {
			initMap(mapTiles.get(k), k);
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
		if (state.pad_lat == 0 && state.pad_lon == 0)
			return;
		if (!state.gps.locked) {
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
		for (Point offset : mapTiles.keySet()) {
			AltosSiteMapTile tile = mapTiles.get(offset);
			Point2D.Double ref, lref;
			ref = translatePoint(pt, tileCoordOffset(offset));
			lref = translatePoint(last_pt, tileCoordOffset(offset));
			tile.show(state, crc_errors, lref, ref);
			if (0 <= ref.x && ref.x < px_size)
				if (0 <= ref.y && ref.y < px_size)
					in_any = true;
		}

		Point offset = tileOffset(pt);
		if (!in_any) {
			Point2D.Double ref, lref;
			ref = translatePoint(pt, tileCoordOffset(offset));
			lref = translatePoint(last_pt, tileCoordOffset(offset));

			AltosSiteMapTile tile = createTile(offset);
			tile.show(state, crc_errors, lref, ref);
			initMap(tile, offset);
			finishTileLater(tile, offset);
		}

		scrollRocketToVisible(pt);

		if (offset != tileOffset(last_pt)) {
			ensureTilesAround(offset);
		}

		last_pt = pt;
		last_state = state.state;
	}

	private AltosSiteMapTile createTile(Point offset) {
		AltosSiteMapTile tile = new AltosSiteMapTile(px_size);
		mapTiles.put(offset, tile);
		return tile;
	}
	private void finishTileLater(final AltosSiteMapTile tile,
				     final Point offset)
	{
		SwingUtilities.invokeLater( new Runnable() {
			public void run() {
				addTileAt(tile, offset);
			}
		} );
	}

	private void ensureTilesAround(Point base_offset) {
		for (int x = -1; x <= 1; x++) {
			for (int y = -1; y <= 1; y++) {
				Point offset = new Point(base_offset.x + x, base_offset.y + y);
				if (mapTiles.containsKey(offset))
					continue;
				AltosSiteMapTile tile = createTile(offset);
				initMap(tile, offset);
				finishTileLater(tile, offset);
			}
		}
	}

	private Point topleft = new Point(0,0);
	private void scrollRocketToVisible(Point2D.Double pt) {
		Rectangle r = comp.getVisibleRect();
		Point2D.Double copt = translatePoint(pt, tileCoordOffset(topleft));
		int dx = (int)copt.x - r.width/2 - r.x;
		int dy = (int)copt.y - r.height/2 - r.y;
		if (Math.abs(dx) > r.width/4 || Math.abs(dy) > r.height/4) {
			r.x += dx;
			r.y += dy;
			comp.scrollRectToVisible(r);
		}
	}

	private void addTileAt(AltosSiteMapTile tile, Point offset) {
		if (Math.abs(offset.x) >= MAX_TILE_DELTA ||
				Math.abs(offset.y) >= MAX_TILE_DELTA)
		{
			System.out.printf("Rocket too far away from pad (tile %d,%d)\n",
					  offset.x, offset.y);
			return;
		}

		boolean review = false;
		Rectangle r = comp.getVisibleRect();
		if (offset.x < topleft.x) {
			r.x += (topleft.x - offset.x) * px_size;
			topleft.x = offset.x;
			review = true;
		}
		if (offset.y < topleft.y) {
			r.y += (topleft.y - offset.y) * px_size;
			topleft.y = offset.y;
			review = true;
		}
		GridBagConstraints c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.BOTH;
		// put some space between the map tiles, debugging only
		// c.insets = new Insets(5, 5, 5, 5);

		c.gridx = offset.x + MAX_TILE_DELTA;
		c.gridy = offset.y + MAX_TILE_DELTA;
		layout.setConstraints(tile, c);

		comp.add(tile);
		if (review) {
			comp.scrollRectToVisible(r);
		}
	}

	private AltosSiteMap(boolean knowWhatYouAreDoing) {
		if (!knowWhatYouAreDoing) {
			throw new RuntimeException("Arggh.");
		}
	}

	JComponent comp = new JComponent() { };
	private GridBagLayout layout = new GridBagLayout();

	public AltosSiteMap() {
		GrabNDrag scroller = new GrabNDrag(comp);

		comp.setLayout(layout);

		for (int x = -1; x <= 1; x++) {
			for (int y = -1; y <= 1; y++) {
				Point offset = new Point(x, y);
				AltosSiteMapTile t = createTile(offset);
				addTileAt(t, offset);
			}
		}
		setViewportView(comp);
		setPreferredSize(new Dimension(500,200));
	}
}

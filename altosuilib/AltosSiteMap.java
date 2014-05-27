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
import java.awt.event.*;
import javax.swing.*;
import java.io.*;
import java.lang.Math;
import java.awt.geom.*;
import java.util.*;
import java.util.concurrent.*;
import org.altusmetrum.altoslib_4.*;

class MapPoint {
	double	lat, lon;
	int	state;

	public MapPoint(double lat, double lon, int state) {
		this.lat = lat;
		this.lon = lon;
		this.state = state;
	}

	public boolean equals(MapPoint other) {
		if (other == null)
			return false;
		if (other.lat != lat)
			return false;
		if (other.lon != lon)
			return false;
		if (other.state != state)
			return false;
		return true;
	}
}

public class AltosSiteMap extends JComponent implements AltosFlightDisplay, MouseMotionListener, MouseListener {
	// preferred vertical step in a tile in naut. miles
	// will actually choose a step size between x and 2x, where this
	// is 1.5x
	static final double tile_size_nmi = 0.75;

	static final int px_size = 512;

	static final int MAX_TILE_DELTA = 100;

	static final int maptype_hybrid = 0;
	static final int maptype_roadmap = 1;
	static final int maptype_satellite = 2;
	static final int maptype_terrain = 3;

	int maptype = maptype_hybrid;

	static final String[] maptype_names = {
		"hybrid",
		"roadmap",
		"satellite",
		"terrain"
	};

	static final String[] maptype_labels = {
		"Hybrid",
		"Roadmap",
		"Satellite",
		"Terrain"
	};

	LinkedList<MapPoint> points = new LinkedList<MapPoint>();

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
	/*
	private static Point2D.Double pt(LatLng latlng, int zoom) {
		double scale_x = 256/360.0 * Math.pow(2, zoom);
		double scale_y = 256/(2.0*Math.PI) * Math.pow(2, zoom);
		return pt(latlng, scale_x, scale_y);
	}
	*/

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

	static final int default_zoom = 15;
	static final int min_zoom = 3;
	static final int max_zoom = 21;

	int zoom = default_zoom;

	double scale_x, scale_y;

	int radius;	/* half width/height of tiles to load */

	private Point2D.Double pt(double lat, double lng) {
		return pt(new LatLng(lat, lng), scale_x, scale_y);
	}

	private LatLng latlng(double x, double y) {
		return latlng(new Point2D.Double(x,y), scale_x, scale_y);
	}
	/*
	private LatLng latlng(Point2D.Double pt) {
		return latlng(pt, scale_x, scale_y);
	}
	*/

	private LatLng latlng(Point pt) {
		return latlng(new Point2D.Double(pt.x, pt.y), scale_x, scale_y);
	}

	ConcurrentHashMap<Point,AltosSiteMapTile> mapTiles = new ConcurrentHashMap<Point,AltosSiteMapTile>();
	Point2D.Double centre;

	private Point2D.Double tileCoordOffset(Point p) {
		return new Point2D.Double(centre.x - p.x*px_size,
					  centre.y - p.y*px_size);
	}

	private Point tileOffset(Point2D.Double p) {
		return new Point((int)Math.floor((centre.x+p.x)/px_size),
				 (int)Math.floor((centre.y+p.y)/px_size));
	}

	private Point2D.Double getBaseLocation(double lat, double lng) {
		Point2D.Double locn = pt(0,0), north_step;

		scale_x = 256/360.0 * Math.pow(2, zoom);
		scale_y = 256/(2.0*Math.PI) * Math.pow(2, zoom);
		locn = pt(lat, lng);
		locn.x = -px_size * Math.floor(locn.x/px_size);
		locn.y = -px_size * Math.floor(locn.y/px_size);
		return locn;
	}

	public void reset() {
		// nothing
	}

	public void set_font() {
		for (AltosSiteMapTile tile : mapTiles.values())
			tile.set_font(AltosUILib.value_font);
	}

	static final int load_mode_cached = 1;
	static final int load_mode_uncached = 2;

	private boolean load_map(final AltosSiteMapTile tile,
				 final File pngfile, String pngurl,
				 int load_mode)
	{
		boolean has_map = AltosSiteMapCache.has_map(pngfile, pngurl);
		if ((load_mode & load_mode_uncached) == 0 && !has_map)
			return false;
		if ((load_mode & load_mode_cached) == 0 && has_map)
			return false;

		tile.set_status(AltosSiteMapCache.loading);
		int status = AltosSiteMapCache.fetch_map(pngfile, pngurl);
		if (status == AltosSiteMapCache.success) {
			SwingUtilities.invokeLater(new Runnable() {
					public void run() {
						tile.load_map(pngfile);
					}
				});
		} else {
			tile.set_status(status);
			System.out.printf("# Failed to fetch file %s (status %d)\n", pngfile, status);
			System.out.printf(" wget -O '%s' '%s'\n", pngfile, pngurl);
			System.out.printf(" sleep 1\n");
		}
		return true;
	}


	class AltosSiteMapPrefetch {
		int	x;
		int	y;
		int	result;
		File	pngfile;
		String	pngurl;
	}

	public AltosSiteMapPrefetch prefetchMap(int x, int y) {
		AltosSiteMapPrefetch	prefetch = new AltosSiteMapPrefetch();
		LatLng map_latlng = latlng(
			-centre.x + x*px_size + px_size/2,
			-centre.y + y*px_size + px_size/2);
		prefetch.pngfile = MapFile(map_latlng.lat, map_latlng.lng, zoom, maptype_hybrid);
		prefetch.pngurl = MapURL(map_latlng.lat, map_latlng.lng, zoom, maptype_hybrid);
		if (AltosSiteMapCache.has_map(prefetch.pngfile, prefetch.pngurl)) {
			prefetch.result = 1;
		} else if (AltosSiteMapCache.fetch_map(prefetch.pngfile, prefetch.pngurl) == AltosSiteMapCache.success) {
			prefetch.result = 0;
		} else {
			prefetch.result = -1;
		}
		return prefetch;
	}

	public static void prefetchMaps(double lat, double lng) {
		int w = AltosSiteMapPreload.width;
		int h = AltosSiteMapPreload.height;
		AltosSiteMap asm = new AltosSiteMap(true);
		asm.centre = asm.getBaseLocation(lat, lng);

		//Point2D.Double p = new Point2D.Double();
		//Point2D.Double p2;
		int dx = -w/2, dy = -h/2;
		for (int y = dy; y < h+dy; y++) {
			for (int x = dx; x < w+dx; x++) {
				AltosSiteMapPrefetch prefetch = asm.prefetchMap(x, y);
				switch (prefetch.result) {
				case 1:
					System.out.printf("Already have %s\n", prefetch.pngfile);
					break;
				case 0:
					System.out.printf("Fetched map %s\n", prefetch.pngfile);
					break;
				case -1:
					System.out.printf("# Failed to fetch file %s\n", prefetch.pngfile);
					System.out.printf(" wget -O '%s' ''\n",
							  prefetch.pngfile, prefetch.pngurl);
					break;
				}
			}
		}
	}

	public File init_map(Point offset, int load_mode) {
		AltosSiteMapTile tile = mapTiles.get(offset);
		Point2D.Double coord = tileCoordOffset(offset);

		LatLng map_latlng = latlng(px_size/2-coord.x, px_size/2-coord.y);

		File pngfile = MapFile(map_latlng.lat, map_latlng.lng, zoom, maptype);
		String pngurl = MapURL(map_latlng.lat, map_latlng.lng, zoom, maptype);
		load_map(tile, pngfile, pngurl, load_mode);
		return pngfile;
	}

	public void initAndFinishMapAsync (final AltosSiteMapTile tile, final Point offset) {
		Thread thread = new Thread() {
				public void run() {
					init_map(offset, load_mode_cached|load_mode_uncached);
					finishTileLater(tile, offset);
				}
			};
		thread.start();
	}

	double	lat, lon;
	boolean base_location_set = false;

	public void clear_base_location() {
		base_location_set = false;
		circle_set = false;
		points = new LinkedList<MapPoint>();
		line_start = line_end = null;
	}

	public void setBaseLocation(double lat, double lng) {
		for (AltosSiteMapTile tile : mapTiles.values())
			tile.clearMap();
		this.lat = lat;
		this.lon = lng;
		base_location_set = true;

		centre = getBaseLocation(lat, lng);
		scrollRocketToVisible(pt(lat,lng));
	}

	private void initMaps(double lat, double lng) {
		setBaseLocation(lat, lng);

		Thread thread = new Thread() {
				public void run() {
					for (Point k : mapTiles.keySet())
						init_map(k, load_mode_cached);
					for (Point k : mapTiles.keySet())
						init_map(k, load_mode_uncached);
				}
			};
		thread.start();
	}

	private static File MapFile(double lat, double lng, int zoom, int maptype) {
		char chlat = lat < 0 ? 'S' : 'N';
		char chlng = lng < 0 ? 'W' : 'E';
		if (lat < 0) lat = -lat;
		if (lng < 0) lng = -lng;
		String maptype_string = String.format("%s-", maptype_names[maptype]);
		String format_string;
		if (maptype == maptype_hybrid || maptype == maptype_satellite || maptype == maptype_terrain)
			format_string = "jpg";
		else
			format_string = "png";
		return new File(AltosUIPreferences.mapdir(),
				String.format("map-%c%.6f,%c%.6f-%s%d.%s",
					      chlat, lat, chlng, lng, maptype_string, zoom, format_string));
	}

	private static String MapURL(double lat, double lng, int zoom, int maptype) {
		String format_string;
		if (maptype == maptype_hybrid || maptype == maptype_satellite || maptype == maptype_terrain)
			format_string = "jpg";
		else
			format_string = "png32";
		return String.format("http://maps.google.com/maps/api/staticmap?center=%.6f,%.6f&zoom=%d&size=%dx%d&sensor=false&maptype=%s&format=%s",
				     lat, lng, zoom, px_size, px_size, maptype_names[maptype], format_string);
	}

	boolean initialised = false;
	MapPoint last_point = null;
	int last_state = -1;

	public void show(double lat, double lon) {
		System.out.printf ("show %g %g\n", lat, lon);
		return;
//		initMaps(lat, lon);
//		scrollRocketToVisible(pt(lat, lon));
	}

	JLabel	zoom_label;

	public void set_zoom_label() {
		zoom_label.setText(String.format("- %d -", zoom - default_zoom));
	}

	public void set_zoom(int zoom) {
		if (min_zoom <= zoom && zoom <= max_zoom) {
			this.zoom = zoom;
			if (base_location_set)
				initMaps(lat, lon);
			redraw();
			set_zoom_label();
		}
	}

	public int get_zoom() {
		return zoom;
	}

	public void draw(MapPoint last_point, MapPoint point) {
		boolean	force_ensure = false;
		if (last_point == null) {
			force_ensure = true;
			last_point = point;
		}

		Point2D.Double pt = pt(point.lat, point.lon);
		Point2D.Double last_pt = pt(last_point.lat, last_point.lon);

		boolean in_any = false;
		for (Point offset : mapTiles.keySet()) {
			AltosSiteMapTile tile = mapTiles.get(offset);
			Point2D.Double ref, lref;
			ref = translatePoint(pt, tileCoordOffset(offset));
			lref = translatePoint(last_pt, tileCoordOffset(offset));
			tile.show(point.state, lref, ref);
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
			tile.show(point.state, lref, ref);
			initAndFinishMapAsync(tile, offset);
		}

		scrollRocketToVisible(pt);

		if (force_ensure || offset != tileOffset(last_pt)) {
			ensureTilesAround(offset);
		}
	}

	public void redraw() {
		MapPoint	last_point = null;

		for (MapPoint point : points) {
			draw(last_point, point);
			last_point = point;
		}
		if (circle_set)
			draw_circle(circle_lat, circle_lon);
		if (line_start != null)
			set_line();
	}

	public void show(final AltosState state, final AltosListenerState listener_state) {
		// if insufficient gps data, nothing to update
		AltosGPS	gps = state.gps;

		if (gps == null)
			return;

		if (!gps.locked && gps.nsat < 4)
			return;

		if (!initialised) {
			if (state.pad_lat != AltosLib.MISSING && state.pad_lon != AltosLib.MISSING) {
				initMaps(state.pad_lat, state.pad_lon);
				initialised = true;
			} else if (gps.lat != AltosLib.MISSING && gps.lon != AltosLib.MISSING) {
				initMaps(gps.lat, gps.lon);
				initialised = true;
			} else {
				return;
			}
		}

		MapPoint	point = new MapPoint(gps.lat, gps.lon, state.state);

		if (point.equals(last_point))
			return;

		points.add(point);

		draw(last_point, point);

		last_point = point;
	}

	public void centre(Point2D.Double pt) {
		Rectangle r = comp.getVisibleRect();
		Point2D.Double copt = translatePoint(pt, tileCoordOffset(topleft));
		int dx = (int)copt.x - r.width/2 - r.x;
		int dy = (int)copt.y - r.height/2 - r.y;
		r.x += dx;
		r.y += dy;
		r.width = 1;
		r.height = 1;
		comp.scrollRectToVisible(r);
	}

	public void centre(AltosState state) {
		if (!state.gps.locked && state.gps.nsat < 4)
			return;
		centre(pt(state.gps.lat, state.gps.lon));
	}

	private double circle_lat, circle_lon;
	private boolean circle_set = false;

	public void draw_circle(double lat, double lon) {
		circle_lat = lat;
		circle_lon = lon;
		circle_set = true;

		Point2D.Double pt = pt(lat, lon);

		for (Point offset : mapTiles.keySet()) {
			AltosSiteMapTile tile = mapTiles.get(offset);
			Point2D.Double ref = translatePoint(pt, tileCoordOffset(offset));
			tile.set_boost(ref);
		}
	}

	private AltosSiteMapTile createTile(Point offset) {
		AltosSiteMapTile tile = new AltosSiteMapTile(px_size);
		tile.set_font(AltosUILib.value_font);
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
		for (int x = -radius; x <= radius; x++) {
			for (int y = -radius; y <= radius; y++) {
				Point offset = new Point(base_offset.x + x, base_offset.y + y);
				if (mapTiles.containsKey(offset))
					continue;
				AltosSiteMapTile tile = createTile(offset);
				initAndFinishMapAsync(tile, offset);
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
//		if (review) {
//			comp.scrollRectToVisible(r);
//		}
	}

	private AltosSiteMap(boolean knowWhatYouAreDoing) {
		if (!knowWhatYouAreDoing) {
			throw new RuntimeException("Arggh.");
		}
	}

	JComponent comp;

	private GridBagLayout layout = new GridBagLayout();

	LatLng	line_start, line_end;

	private void set_line() {
		if (line_start != null && line_end != null) {
			Point2D.Double	start = pt(line_start.lat, line_start.lng);
			Point2D.Double	end = pt(line_end.lat, line_end.lng);
			AltosGreatCircle	g = new AltosGreatCircle(line_start.lat, line_start.lng,
									 line_end.lat, line_end.lng);

			for (Point offset : mapTiles.keySet()) {
				AltosSiteMapTile tile = mapTiles.get(offset);
				Point2D.Double s, e;
				s = translatePoint(start, tileCoordOffset(offset));
				e = translatePoint(end, tileCoordOffset(offset));
				tile.set_line(new Line2D.Double(s.x, s.y, e.x, e.y), g.distance);
			}
		} else {
			for (AltosSiteMapTile tile : mapTiles.values())
				tile.set_line(null, 0);
		}
	}

	LatLng latlng(MouseEvent e) {
		if (!base_location_set)
			return null;

		Rectangle	zerozero = mapTiles.get(new Point(0, 0)).getBounds();

		return latlng(-centre.x + e.getPoint().x - zerozero.x, -centre.y + e.getPoint().y - zerozero.y);
	}

	/* MouseMotionListener methods */
	public void mouseDragged(MouseEvent e) {
		if (!GrabNDrag.grab_n_drag(e)) {
			LatLng	loc = latlng(e);
			line_end = loc;
			set_line();
		}
	}

	public void mouseMoved(MouseEvent e) {
	}

	/* MouseListener methods */
	public void mouseClicked(MouseEvent e) {
	}

	public void mouseEntered(MouseEvent e) {
	}

	public void mouseExited(MouseEvent e) {
	}

	public void mousePressed(MouseEvent e) {
		if (!GrabNDrag.grab_n_drag(e)) {
			LatLng	loc = latlng(e);
			line_start = loc;
			line_end = null;
			set_line();
		}
	}

	public void mouseReleased(MouseEvent e) {
	}

	JScrollPane	pane = new JScrollPane();

	public AltosSiteMap(int in_radius) {
		radius = in_radius;

		comp = new JComponent() { };

		comp.addMouseMotionListener(this);
		comp.addMouseListener(this);

		GrabNDrag scroller = new GrabNDrag(comp);

		comp.setLayout(layout);

		for (int x = -radius; x <= radius; x++) {
			for (int y = -radius; y <= radius; y++) {
				Point offset = new Point(x, y);
				AltosSiteMapTile t = createTile(offset);
				addTileAt(t, offset);
			}
		}
		pane.setViewportView(comp);
		pane.setPreferredSize(new Dimension(500,500));
		pane.setVisible(true);
		pane.setEnabled(true);

		GridBagLayout	my_layout = new GridBagLayout();

		setLayout(my_layout);

		GridBagConstraints c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.BOTH;
		c.gridx = 0;
		c.gridy = 0;
		c.gridwidth = 1;
		c.gridheight = 10;
		c.weightx = 1;
		c.weighty = 1;
		add(pane, c);

		int	y = 0;

		zoom_label = new JLabel("", JLabel.CENTER);
		set_zoom_label();

		c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridx = 1;
		c.gridy = y++;
		c.weightx = 0;
		c.weighty = 0;
		add(zoom_label, c);

		JButton zoom_reset = new JButton("0");
		zoom_reset.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					set_zoom(default_zoom);
				}
			});

		c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridx = 1;
		c.gridy = y++;
		c.weightx = 0;
		c.weighty = 0;
		add(zoom_reset, c);

		JButton zoom_in = new JButton("+");
		zoom_in.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					set_zoom(get_zoom() + 1);
				}
			});

		c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridx = 1;
		c.gridy = y++;
		c.weightx = 0;
		c.weighty = 0;
		add(zoom_in, c);

		JButton zoom_out = new JButton("-");
		zoom_out.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					set_zoom(get_zoom() - 1);
				}
			});
		c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridx = 1;
		c.gridy = y++;
		c.weightx = 0;
		c.weighty = 0;
		add(zoom_out, c);

		final JComboBox<String>	maptype_combo = new JComboBox<String>(maptype_labels);

		maptype_combo.setEditable(false);
		maptype_combo.setMaximumRowCount(maptype_combo.getItemCount());
		maptype_combo.addItemListener(new ItemListener() {
				public void itemStateChanged(ItemEvent e) {
					maptype = maptype_combo.getSelectedIndex();
					if (base_location_set)
						initMaps(lat, lon);
					redraw();
				}
			});

		c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridx = 1;
		c.gridy = y++;
		c.weightx = 0;
		c.weighty = 0;
		add(maptype_combo, c);
	}

	public AltosSiteMap() {
		this(1);
	}
}

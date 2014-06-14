/*
 * Copyright © 2014 Keith Packard <keithp@keithp.com>
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
import java.awt.image.*;
import javax.swing.*;
import java.io.*;
import java.lang.*;
import java.awt.geom.*;
import java.util.*;
import java.util.concurrent.*;
import org.altusmetrum.altoslib_4.*;

public class AltosUIMapView extends Component implements MouseMotionListener, MouseListener, MouseWheelListener, ComponentListener, AltosUIMapTileListener, AltosUIMapStoreListener {

	AltosUIMapPath	path = new AltosUIMapPath();

	AltosUIMapLine	line = new AltosUIMapLine();

	AltosUIMapCache	cache = new AltosUIMapCache();

	LinkedList<AltosUIMapMark> marks = new LinkedList<AltosUIMapMark>();

	LinkedList<AltosUIMapZoomListener> zoom_listeners = new LinkedList<AltosUIMapZoomListener>();

	boolean		have_boost = false;
	boolean		have_landed = false;

	ConcurrentHashMap<Point,AltosUIMapTile> tiles = new ConcurrentHashMap<Point,AltosUIMapTile>();

	static final int default_zoom = 15;
	static final int min_zoom = 3;
	static final int max_zoom = 21;
	static final int px_size = 512;

	int		load_radius;
	AltosUILatLon	load_centre = null;
	AltosUIMapTileListener	load_listener;

	int 		zoom = default_zoom;
	int		maptype = AltosUIMap.maptype_default;

	long		user_input_time;

	/* Milliseconds to wait after user action before auto-scrolling
	 */
	static final long auto_scroll_delay = 20 * 1000;

	AltosUIMapTransform	transform;
	AltosUILatLon		centre;

	public void set_font() {
		line.set_font(AltosUILib.value_font);
		for (AltosUIMapTile tile : tiles.values())
			tile.set_font(AltosUILib.value_font);
		repaint();
	}

	public void set_units() {
		repaint();
	}

	private boolean is_drag_event(MouseEvent e) {
		return e.getModifiers() == InputEvent.BUTTON1_MASK;
	}

	Point	drag_start;

	private void drag(MouseEvent e) {
		if (drag_start == null)
			return;

		int dx = e.getPoint().x - drag_start.x;
		int dy = e.getPoint().y - drag_start.y;

		AltosUILatLon	new_centre = transform.screen_lat_lon(new Point(getWidth() / 2 - dx, getHeight() / 2 - dy));
		centre (new_centre.lat, new_centre.lon);
		drag_start = e.getPoint();
	}

	private void drag_start(MouseEvent e) {
		drag_start = e.getPoint();
	}

	private void notice_user_input() {
		user_input_time = System.currentTimeMillis();
	}

	private boolean recent_user_input() {
		return (System.currentTimeMillis() - user_input_time) < auto_scroll_delay;
	}

	/* MouseMotionListener methods */

	public void mouseDragged(MouseEvent e) {
		notice_user_input();
		if (is_drag_event(e))
			drag(e);
		else {
			line.dragged(e, transform);
			repaint();
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
		notice_user_input();
		if (is_drag_event(e))
			drag_start(e);
		else {
			line.pressed(e, transform);
			repaint();
		}
	}

	public void mouseReleased(MouseEvent e) {
	}

	/* MouseWheelListener methods */

	public void mouseWheelMoved(MouseWheelEvent e) {
		int	zoom_change = e.getWheelRotation();

		notice_user_input();
		AltosUILatLon	mouse_lat_lon = transform.screen_lat_lon(e.getPoint());
		set_zoom(zoom() - zoom_change);

		Point2D.Double	new_mouse = transform.screen(mouse_lat_lon);

		int	dx = getWidth()/2 - e.getPoint().x;
		int	dy = getHeight()/2 - e.getPoint().y;

		AltosUILatLon	new_centre = transform.screen_lat_lon(new Point((int) new_mouse.x + dx, (int) new_mouse.y + dy));

		centre(new_centre.lat, new_centre.lon);
	}

	/* ComponentListener methods */

	public void componentHidden(ComponentEvent e) {
	}

	public void componentMoved(ComponentEvent e) {
	}

	public void componentResized(ComponentEvent e) {
		set_transform();
	}

	public void componentShown(ComponentEvent e) {
		set_transform();
	}

	public void repaint(Rectangle r, int pad) {
		repaint(r.x - pad, r.y - pad, r.width + pad*2, r.height + pad*2);
	}

	public void repaint(AltosUIMapRectangle rect, int pad) {
		repaint (transform.screen(rect), pad);
	}

	private boolean far_from_centre(AltosUILatLon lat_lon) {

		if (centre == null || transform == null)
			return true;

		Point2D.Double	screen = transform.screen(lat_lon);

		int		width = getWidth();
		int		dx = Math.abs ((int) screen.x - width/2);

		if (dx > width / 4)
			return true;

		int		height = getHeight();
		int		dy = Math.abs ((int) screen.y - height/2);

		if (dy > height / 4)
			return true;

		return false;
	}

	public void show(AltosState state, AltosListenerState listener_state) {

		/* If insufficient gps data, nothing to update
		 */
		AltosGPS	gps = state.gps;

		if (gps == null)
			return;

		if (!gps.locked && gps.nsat < 4)
			return;

		AltosUIMapRectangle	damage = path.add(gps.lat, gps.lon, state.state);

		switch (state.state) {
		case AltosLib.ao_flight_boost:
			if (!have_boost) {
				add_mark(gps.lat, gps.lon, state.state);
				have_boost = true;
			}
			break;
		case AltosLib.ao_flight_landed:
			if (!have_landed) {
				add_mark(gps.lat, gps.lon, state.state);
				have_landed = true;
			}
			break;
		}

		if (damage != null)
			repaint(damage, AltosUIMapPath.stroke_width);
		maybe_centre(gps.lat, gps.lon);
	}

	private void set_transform() {
		Rectangle	bounds = getBounds();

		transform = new AltosUIMapTransform(bounds.width, bounds.height, zoom, centre);
		repaint();
	}

	public boolean set_zoom(int zoom) {
		if (min_zoom <= zoom && zoom <= max_zoom && zoom != this.zoom) {
			this.zoom = zoom;
			tiles.clear();
			set_transform();

			for (AltosUIMapZoomListener listener : zoom_listeners)
				listener.zoom_changed(this.zoom);

			return true;
		}
		return false;
	}

	public void add_zoom_listener(AltosUIMapZoomListener listener) {
		if (!zoom_listeners.contains(listener))
			zoom_listeners.add(listener);
	}

	public void remove_zoom_listener(AltosUIMapZoomListener listener) {
		zoom_listeners.remove(listener);
	}

	public void set_load_params(double lat, double lon, int radius, AltosUIMapTileListener listener) {
		load_centre = new AltosUILatLon(lat, lon);
		load_radius = radius;
		load_listener = listener;
		centre(lat, lon);
		make_tiles();
		for (AltosUIMapTile tile : tiles.values()) {
			tile.add_store_listener(this);
			if (tile.store_status() != AltosUIMapStore.loading)
				listener.notify_tile(tile, tile.store_status());
		}
		repaint();
	}

	public boolean all_fetched() {
		for (AltosUIMapTile tile : tiles.values()) {
			if (tile.store_status() == AltosUIMapStore.loading)
				return false;
		}
		return true;
	}

	public boolean set_maptype(int maptype) {
		if (maptype != this.maptype) {
			this.maptype = maptype;
			tiles.clear();
			repaint();
			return true;
		}
		return false;
	}

	public int get_maptype() {
		return maptype;
	}

	public int zoom() {
		return zoom;
	}

	public void centre(AltosUILatLon lat_lon) {
		centre = lat_lon;
		set_transform();
	}

	public void centre(double lat, double lon) {
		centre(new AltosUILatLon(lat, lon));
	}

	public void maybe_centre(double lat, double lon) {
		AltosUILatLon	lat_lon = new AltosUILatLon(lat, lon);
		if (centre == null || (!recent_user_input() && far_from_centre(lat_lon)))
			centre(lat_lon);
	}

	private VolatileImage create_back_buffer() {
		return getGraphicsConfiguration().createCompatibleVolatileImage(getWidth(), getHeight());
	}

	private Point floor(Point2D.Double point) {
		return new Point ((int) Math.floor(point.x / px_size) * px_size,
				  (int) Math.floor(point.y / px_size) * px_size);
	}

	private Point ceil(Point2D.Double point) {
		return new Point ((int) Math.ceil(point.x / px_size) * px_size,
				  (int) Math.ceil(point.y / px_size) * px_size);
	}

	private void make_tiles() {
		Point	upper_left;
		Point	lower_right;

		if (load_centre != null) {
			Point centre = floor(transform.point(load_centre));

			upper_left = new Point(centre.x - load_radius * px_size,
					       centre.y - load_radius * px_size);
			lower_right = new Point(centre.x + load_radius * px_size,
					       centre.y + load_radius * px_size);
		} else {
			upper_left = floor(transform.screen_point(new Point(0, 0)));
			lower_right = floor(transform.screen_point(new Point(getWidth(), getHeight())));
		}
		LinkedList<Point> to_remove = new LinkedList<Point>();

		for (Point point : tiles.keySet()) {
			if (point.x < upper_left.x || lower_right.x < point.x ||
			    point.y < upper_left.y || lower_right.y < point.y) {
				to_remove.add(point);
			}
		}

		for (Point point : to_remove)
			tiles.remove(point);

		cache.set_cache_size(((lower_right.y - upper_left.y) / px_size + 1) * ((lower_right.x - upper_left.x) / px_size + 1));
		for (int y = upper_left.y; y <= lower_right.y; y += px_size) {
			for (int x = upper_left.x; x <= lower_right.x; x += px_size) {
				Point point = new Point(x, y);

				if (!tiles.containsKey(point)) {
					AltosUILatLon	ul = transform.lat_lon(new Point2D.Double(x, y));
					AltosUILatLon	center = transform.lat_lon(new Point2D.Double(x + px_size/2, y + px_size/2));
					AltosUIMapTile tile = new AltosUIMapTile(this, ul, center, zoom, maptype,
										 px_size, AltosUILib.value_font);
					tiles.put(point, tile);
				}
			}
		}
	}

	/* AltosUIMapTileListener methods */
	public void notify_tile(AltosUIMapTile tile, int status) {
		for (Point point : tiles.keySet()) {
			if (tile == tiles.get(point)) {
				Point	screen = transform.screen(point);
				repaint(screen.x, screen.y, px_size, px_size);
			}
		}
	}

	public AltosUIMapCache cache() { return cache; }

	/* AltosUIMapStoreListener methods */
	public void notify_store(AltosUIMapStore store, int status) {
		if (load_listener != null) {
			for (AltosUIMapTile tile : tiles.values())
				if (store.equals(tile.store))
					load_listener.notify_tile(tile, status);
		}
	}

	private void do_paint(Graphics g) {
		Graphics2D	g2d = (Graphics2D) g;

		make_tiles();

		for (AltosUIMapTile tile : tiles.values())
			tile.paint(g2d, transform);

		synchronized(marks) {
			for (AltosUIMapMark mark : marks)
				mark.paint(g2d, transform);
		}

		path.paint(g2d, transform);

		line.paint(g2d, transform);
	}

	public void paint(Graphics g) {
		VolatileImage	back_buffer = create_back_buffer();
		do {
			GraphicsConfiguration gc = getGraphicsConfiguration();
			int code = back_buffer.validate(gc);
			if (code == VolatileImage.IMAGE_INCOMPATIBLE)
				back_buffer = create_back_buffer();

			Graphics g_back = back_buffer.getGraphics();
			g_back.setClip(g.getClip());
			do_paint(g_back);
			g_back.dispose();

			g.drawImage(back_buffer, 0, 0, this);
		} while (back_buffer.contentsLost());
		back_buffer.flush();
	}

	public void update(Graphics g) {
		paint(g);
	}

	public void add_mark(double lat, double lon, int state) {
		synchronized(marks) {
			marks.add(new AltosUIMapMark(lat, lon, state));
		}
		repaint();
	}

	public void clear_marks() {
		synchronized(marks) {
			marks.clear();
		}
	}

	public AltosUIMapView() {
		centre(0, 0);

		addComponentListener(this);
		addMouseMotionListener(this);
		addMouseListener(this);
		addMouseWheelListener(this);
		set_font();
	}
}

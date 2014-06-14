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
import java.awt.image.*;
import javax.swing.*;
import javax.imageio.*;
import java.awt.geom.*;
import java.io.*;
import java.util.*;
import java.awt.RenderingHints.*;
import org.altusmetrum.altoslib_4.*;

public class AltosUIMapTile {
	AltosUIMapTileListener	listener;
	AltosUILatLon	upper_left, center;
	int		px_size;
	int		zoom;
	int		maptype;
	AltosUIMapStore	store;
	AltosUIMapCache	cache;
	int		status;

	private File map_file() {
		double lat = center.lat;
		double lon = center.lon;
		char chlat = lat < 0 ? 'S' : 'N';
		char chlon = lon < 0 ? 'W' : 'E';

		if (lat < 0) lat = -lat;
		if (lon < 0) lon = -lon;
		String maptype_string = String.format("%s-", AltosUIMap.maptype_names[maptype]);
		String format_string;
		if (maptype == AltosUIMap.maptype_hybrid || maptype == AltosUIMap.maptype_satellite || maptype == AltosUIMap.maptype_terrain)
			format_string = "jpg";
		else
			format_string = "png";
		return new File(AltosUIPreferences.mapdir(),
				String.format("map-%c%.6f,%c%.6f-%s%d.%s",
					      chlat, lat, chlon, lon, maptype_string, zoom, format_string));
	}

	private String map_url() {
		String format_string;
		if (maptype == AltosUIMap.maptype_hybrid || maptype == AltosUIMap.maptype_satellite || maptype == AltosUIMap.maptype_terrain)
			format_string = "jpg";
		else
			format_string = "png32";

		if (AltosUIVersion.has_google_maps_api_key())
			return String.format("http://maps.google.com/maps/api/staticmap?center=%.6f,%.6f&zoom=%d&size=%dx%d&sensor=false&maptype=%s&format=%s&key=%s",
					     center.lat, center.lon, zoom, px_size, px_size, AltosUIMap.maptype_names[maptype], format_string, AltosUIVersion.google_maps_api_key);
		else
			return String.format("http://maps.google.com/maps/api/staticmap?center=%.6f,%.6f&zoom=%d&size=%dx%d&sensor=false&maptype=%s&format=%s",
					     center.lat, center.lon, zoom, px_size, px_size, AltosUIMap.maptype_names[maptype], format_string);
	}
	private Font	font = null;

	public void set_font(Font font) {
		this.font = font;
	}

	int	painting_serial;
	int	painted_serial;

	Image	image;

	public void paint_graphics(Graphics2D g2d, AltosUIMapTransform t, int serial) {
		if (serial < painted_serial)
			return;

		Point2D.Double	point_double = t.screen(upper_left);
		Point		point = new Point((int) (point_double.x + 0.5),
						  (int) (point_double.y + 0.5));

		painted_serial = serial;

		if (!g2d.hitClip(point.x, point.y, px_size, px_size))
			return;

		if (image != null) {
			g2d.drawImage(image, point.x, point.y, null);
			image = null;
		} else {
			g2d.setColor(Color.GRAY);
			g2d.fillRect(point.x, point.y, px_size, px_size);

			if (t.has_location()) {
				String	message = null;
				switch (status) {
				case AltosUIMapCache.loading:
					message = "Loading...";
					break;
				case AltosUIMapCache.bad_request:
					message = "Internal error";
					break;
				case AltosUIMapCache.failed:
					message = "Network error, check connection";
					break;
				case AltosUIMapCache.forbidden:
					message = "Too many requests, try later";
					break;
				}
				if (message != null && font != null) {
					g2d.setFont(font);
					g2d.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
					Rectangle2D bounds = font.getStringBounds(message, g2d.getFontRenderContext());

					float x = px_size / 2.0f;
					float y = px_size / 2.0f;
					x = x - (float) bounds.getWidth() / 2.0f;
					y = y + (float) bounds.getHeight() / 2.0f;
					g2d.setColor(Color.BLACK);
					g2d.drawString(message, (float) point_double.x + x, (float) point_double.y + y);
				}
			}
		}
	}

	public void set_status(int status) {
		this.status = status;
		listener.notify_tile(this, status);
	}

	public void notify_image(Image image) {
		listener.notify_tile(this, status);
	}

	public void paint(Graphics g, AltosUIMapTransform t) {
		Graphics2D		g2d = (Graphics2D) g;
		boolean			queued = false;

		Point2D.Double	point = t.screen(upper_left);

		if (!g.hitClip((int) (point.x + 0.5), (int) (point.y + 0.5), px_size, px_size))
			return;

		++painting_serial;

		if (image == null && t.has_location())
			image = cache.get(this, store, px_size, px_size);

		paint_graphics(g2d, t, painting_serial);
	}

	public int store_status() {
		return store.status();
	}

	public void add_store_listener(AltosUIMapStoreListener listener) {
		store.add_listener(listener);
	}

	public void remove_store_listener(AltosUIMapStoreListener listener) {
		store.remove_listener(listener);
	}

	public AltosUIMapTile(AltosUIMapTileListener listener, AltosUILatLon upper_left, AltosUILatLon center, int zoom, int maptype, int px_size, Font font) {
		this.listener = listener;
		this.upper_left = upper_left;
		cache = listener.cache();

		while (center.lon < -180.0)
			center.lon += 360.0;
		while (center.lon > 180.0)
			center.lon -= 360.0;

		this.center = center;
		this.zoom = zoom;
		this.maptype = maptype;
		this.px_size = px_size;
		this.font = font;
		status = AltosUIMapCache.loading;
		store = AltosUIMapStore.get(map_url(), map_file());
	}
}

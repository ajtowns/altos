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

import javax.swing.*;
import javax.imageio.ImageIO;
import java.awt.image.*;
import java.awt.*;
import java.io.*;
import java.net.*;

public class AltosUIMapImage implements AltosUIMapStoreListener {
	static final long google_maps_ratelimit_ms = 1200;
	// Google limits static map queries to 50 per minute per IP, so
	// each query should take at least 1.2 seconds.

	static final int	success = 0;
	static final int	loading = 1;
	static final int	failed = 2;
	static final int	bad_request = 3;
	static final int	forbidden = 4;

	static long		forbidden_time;
	static boolean		forbidden_set = false;
	static final long	forbidden_interval = 60l * 1000l * 1000l * 1000l;

	AltosUIMapTile		tile;		/* Notify when image has been loaded */
	Image			image;
	AltosUIMapStore		store;
	long			used;

	class loader implements Runnable {
		public void run() {
			if (image != null)
				tile.notify_image(image);
			try {
				image = ImageIO.read(store.file);
			} catch (Exception ex) {
			}
			if (image == null)
				tile.set_status(failed);
			else
				tile.set_status(success);
			tile.notify_image(image);
		}
	}

	private void load() {
		loader	l = new loader();
		Thread	lt = new Thread(l);
		lt.start();
	}

	public void flush() {
		if (image != null) {
			image.flush();
			image = null;
		}
	}

	public boolean has_map() {
		return store.status() == AltosUIMapStore.success;
	}

	public synchronized void notify_store(AltosUIMapStore store, int status) {
		switch (status) {
		case AltosUIMapStore.loading:
			break;
		case AltosUIMapStore.success:
			load();
			break;
		default:
			tile.set_status(status);
			tile.notify_image(null);
		}
	}

	public AltosUIMapImage(AltosUIMapTile tile, AltosUIMapStore store) throws IOException {
		this.tile = tile;
		this.image = null;
		this.store = store;
		this.used = 0;

		int status = store.status();
		switch (status) {
		case AltosUIMapStore.loading:
			store.add_listener(this);
			break;
		case AltosUIMapStore.success:
			load();
			break;
		default:
			tile.set_status(status);
			tile.notify_image(null);
			break;
		}
	}
}

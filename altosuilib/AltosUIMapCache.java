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

public class AltosUIMapCache {
	static final int	success = 0;
	static final int	loading = 1;
	static final int	failed = 2;
	static final int	bad_request = 3;
	static final int	forbidden = 4;

	static final int	min_cache_size = 9;
	static final int	max_cache_size = 24;

	private Object 		fetch_lock = new Object();
	private Object 		cache_lock = new Object();

	int			cache_size = min_cache_size;

	AltosUIMapImage[]	images = new AltosUIMapImage[cache_size];

	long			used;

	public void set_cache_size(int new_size) {
		if (new_size < min_cache_size)
			new_size = min_cache_size;
		if (new_size > max_cache_size)
			new_size = max_cache_size;
		if (new_size == cache_size)
			return;

		synchronized(cache_lock) {
			AltosUIMapImage[]	new_images = new AltosUIMapImage[new_size];

			for (int i = 0; i < cache_size; i++) {
				if (i < new_size)
					new_images[i] = images[i];
				else if (images[i] != null)
					images[i].flush();
			}
			images = new_images;
			cache_size = new_size;
		}
	}

	public Image get(AltosUIMapTile tile, AltosUIMapStore store, int width, int height) {
		int		oldest = -1;
		long		age = used;

		synchronized(cache_lock) {
			AltosUIMapImage	image = null;
			for (int i = 0; i < cache_size; i++) {
				image = images[i];

				if (image == null) {
					oldest = i;
					break;
				}
				if (store.equals(image.store)) {
					image.used = used++;
					return image.image;
				}
				if (image.used < age) {
					oldest = i;
					age = image.used;
				}
			}

			try {
				image = new AltosUIMapImage(tile, store);
				image.used = used++;
				if (images[oldest] != null)
					images[oldest].flush();

				images[oldest] = image;

				if (image.image == null)
					tile.set_status(loading);
				else
					tile.set_status(success);

				return image.image;
			} catch (IOException e) {
				tile.set_status(failed);
				return null;
			}
		}
	}

	public AltosUIMapCache() {
	}
}

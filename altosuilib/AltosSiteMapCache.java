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

public class AltosSiteMapCache {
	static final long google_maps_ratelimit_ms = 1200;
	// Google limits static map queries to 50 per minute per IP, so
	// each query should take at least 1.2 seconds.

	static final int	success = 0;
	static final int	loading = 1;
	static final int	failed = 2;
	static final int	bad_request = 3;
	static final int	forbidden = 4;

	public static boolean has_map(File file, String url) {
		return file.exists();
	}

	static long	forbidden_time;
	static boolean	forbidden_set = false;
	static final long	forbidden_interval = 60l * 1000l * 1000l * 1000l;

	static private Object fetch_lock = new Object();

	public static int fetch_map(File file, String url) {
		if (file.exists())
			return success;

		if (forbidden_set && (System.nanoTime() - forbidden_time) < forbidden_interval)
			return forbidden;

		synchronized (fetch_lock) {
			URL u;
			long startTime = System.nanoTime();

			try {
				u = new URL(url);
			} catch (java.net.MalformedURLException e) {
				return bad_request;
			}

			byte[] data;
			URLConnection uc = null;
			try {
				uc = u.openConnection();
				String type = uc.getContentType();
				int contentLength = uc.getContentLength();
				if (uc instanceof HttpURLConnection) {
					int response = ((HttpURLConnection) uc).getResponseCode();
					switch (response) {
					case HttpURLConnection.HTTP_FORBIDDEN:
					case HttpURLConnection.HTTP_PAYMENT_REQUIRED:
					case HttpURLConnection.HTTP_UNAUTHORIZED:
						forbidden_time = System.nanoTime();
						forbidden_set = true;
						return forbidden;
					}
				}
				InputStream in = new BufferedInputStream(uc.getInputStream());
				int bytesRead = 0;
				int offset = 0;
				data = new byte[contentLength];
				while (offset < contentLength) {
					bytesRead = in.read(data, offset, data.length - offset);
					if (bytesRead == -1)
						break;
					offset += bytesRead;
				}
				in.close();

				if (offset != contentLength)
					return failed;

			} catch (IOException e) {
				return failed;
			}

			try {
				FileOutputStream out = new FileOutputStream(file);
				out.write(data);
				out.flush();
				out.close();
			} catch (FileNotFoundException e) {
				return bad_request;
			} catch (IOException e) {
				if (file.exists())
					file.delete();
				return bad_request;
			}

			long duration_ms = (System.nanoTime() - startTime) / 1000000;
			if (duration_ms < google_maps_ratelimit_ms) {
				try {
					Thread.sleep(google_maps_ratelimit_ms - duration_ms);
				} catch (InterruptedException e) {
					Thread.currentThread().interrupt();
				}
			}
			return success;
		}
	}

	static final int		min_cache_size = 9;
	static final int		max_cache_size = 24;

	static int			cache_size = min_cache_size;

	static AltosSiteMapImage[]	images = new AltosSiteMapImage[cache_size];

	static Object cache_lock = new Object();

	public  static void set_cache_size(int new_size) {
		if (new_size < min_cache_size)
			new_size = min_cache_size;
		if (new_size > max_cache_size)
			new_size = max_cache_size;
		if (new_size == cache_size)
			return;

		synchronized(cache_lock) {
			AltosSiteMapImage[]	new_images = new AltosSiteMapImage[new_size];

			for (int i = 0; i < cache_size; i++) {
				if (i < new_size)
					new_images[i] = images[i];
				else
					images[i].flush();
			}
			images = new_images;
			cache_size = new_size;
		}
	}

	static long			used;

	private static Point tile_loc(AltosSiteMapTile tile) {
		Rectangle	r = tile.getBounds();
		int		x = r.x / 512;
		int		y = r.y / 512;

		return new Point (x, y);
	}

	private static void dump_cache() {
		int	min_x = 1000, max_x = -1000, min_y = 1000, max_y = -1000;

		for (int i = 0; i < cache_size; i++) {
			AltosSiteMapImage	image = images[i];
			if (image != null) {
				Point p = tile_loc(image.tile);
				min_x = min_x < p.x ? min_x : p.x;
				max_x = max_x > p.x ? max_x : p.x;
				min_y = min_y < p.y ? min_y : p.y;
				max_y = max_y > p.y ? max_y : p.y;
				System.out.printf ("entry %d %d,%d used %d\n", i, p.x, p.y, image.used);
			} else {
				System.out.printf ("entry %d empty\n", i);
			}
		}

		int[][]	map = new int[max_x - min_x + 1][max_y - min_y + 1];
		for (int i = 0; i < cache_size; i++) {
			AltosSiteMapImage	image = images[i];
			if (image != null) {
				Point p = tile_loc(image.tile);
				map[p.x - min_x][p.y - min_y]++;
			}
		}

		for (int y = min_y; y <= max_y; y++) {
			for (int x = min_x; x <= max_x; x++)
				System.out.printf (" %2d", map[x - min_x][y - min_y]);
			System.out.printf("\n");
		}
	}

	public static AltosSiteMapImage get_image(AltosSiteMapTile tile, File file, int width, int height) {
		int		oldest = -1;
		long		age = used;

		synchronized(cache_lock) {
			AltosSiteMapImage	image = null;
			for (int i = 0; i < cache_size; i++) {
				image = images[i];

				if (image == null) {
					oldest = i;
					break;
				}
				if (image.tile == tile && file.equals(image.file)) {
					image.used = used++;
					return image;
				}
				if (image.used < age) {
					oldest = i;
					age = image.used;
				}
			}

			try {
				image = new AltosSiteMapImage(tile, file, width, height);
				image.used = used++;
				if (images[oldest] != null) {
//					dump_cache();
					AltosSiteMap.debug_component(images[oldest].tile, "replacing cache");
					AltosSiteMap.debug_component(tile, "replaced cache");
					images[oldest].flush();
				}
				images[oldest] = image;
				return image;
			} catch (IOException e) {
				return null;
			}
		}
	}
}

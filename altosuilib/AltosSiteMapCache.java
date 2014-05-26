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
import java.net.URL;
import java.net.URLConnection;


class AltosCacheImage {
	Component	component;
	File		file;
	VolatileImage	image;
	int		width;
	int		height;
	long		used;

	public void load_image() throws IOException {
		BufferedImage	bimg = ImageIO.read(file);
		Graphics2D	g = image.createGraphics();
		g.drawImage(bimg, 0, 0, null);
		bimg.flush();
	}

	public Image validate() {
		int	returnCode;

		if (image != null)
			returnCode = image.validate(component.getGraphicsConfiguration());
		else
			returnCode = VolatileImage.IMAGE_INCOMPATIBLE;
		if (returnCode == VolatileImage.IMAGE_RESTORED) {
			try {
				load_image();
			} catch (IOException e) {
				return null;
			}
		} else if (returnCode == VolatileImage.IMAGE_INCOMPATIBLE) {
			image = component.createVolatileImage(width, height);
			try {
				load_image();
			} catch (IOException e) {
				return null;
			}
		}
		return image;
	}

	public void flush() {
		image.flush();
	}

	public AltosCacheImage(Component component, File file, int w, int h) throws IOException {
		this.component = component;
		this.file = file;
		width = w;
		height = h;
		image = component.createVolatileImage(w, h);
		used = 0;
	}
}

public class AltosSiteMapCache extends JLabel {
	static final long google_maps_ratelimit_ms = 1200;
	// Google limits static map queries to 50 per minute per IP, so
	// each query should take at least 1.2 seconds.

	public static boolean fetchMap(File file, String url) {
		if (file.exists())
			return true;

		URL u;
		long startTime = System.nanoTime();

		try {
			u = new URL(url);
		} catch (java.net.MalformedURLException e) {
			return false;
		}

		byte[] data;
		try {
			URLConnection uc = u.openConnection();
			int contentLength = uc.getContentLength();
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

			if (offset != contentLength) {
				return false;
			}
		} catch (IOException e) {
			return false;
		}

		try {
			FileOutputStream out = new FileOutputStream(file);
			out.write(data);
			out.flush();
			out.close();
		} catch (FileNotFoundException e) {
			return false;
		} catch (IOException e) {
			if (file.exists()) {
				file.delete();
			}
			return false;
		}

		long duration_ms = (System.nanoTime() - startTime) / 1000000;
		if (duration_ms < google_maps_ratelimit_ms) {
			try {
				Thread.sleep(google_maps_ratelimit_ms - duration_ms);
			} catch (InterruptedException e) {
				Thread.currentThread().interrupt();
			}
		}

		return true;
	}

	static int			cache_size = 9;

	static AltosCacheImage[]	images;

	static long			used;

	public static void set_cache_size(int cache_size) {
		AltosSiteMapCache.cache_size = cache_size;
		images = null;
	}

	public static Image get_image(Component component, File file, int width, int height) {
		int		oldest = -1;
		long		age = used;
		AltosCacheImage	image;
		if (images == null)
			images = new AltosCacheImage[cache_size];
		for (int i = 0; i < cache_size; i++) {
			image = images[i];

			if (image == null) {
				oldest = i;
				break;
			}
			if (image.component == component && file.equals(image.file)) {
				image.used = used++;
				return image.validate();
			}
			if (image.used < age) {
				oldest = i;
				age = image.used;
			}
		}

		try {
			image = new AltosCacheImage(component, file, width, height);
			image.used = used++;
			if (images[oldest] != null)
				images[oldest].flush();
			images[oldest] = image;
			return image.validate();
		} catch (IOException e) {
			return null;
		}
	}
}

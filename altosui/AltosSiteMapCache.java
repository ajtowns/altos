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

import javax.swing.*;
import javax.imageio.ImageIO;
import java.awt.image.*;
import java.io.*;
import java.net.URL;
import java.net.URLConnection;

public class AltosSiteMapCache extends JLabel {
	static final long google_maps_ratelimit_ms = 1200;
	// Google limits static map queries to 50 per minute per IP, so
	// each query should take at least 1.2 seconds.

	public static boolean fetchMap(File file, String url) {
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

	public static ImageIcon fetchAndLoadMap(File pngfile, String url) {
		if (!pngfile.exists()) {
			if (!fetchMap(pngfile, url)) {
				return null;
			}
		}
		return loadMap(pngfile, url);
	}

	public static ImageIcon loadMap(File pngfile, String url) {
		if (!pngfile.exists()) {
			return null;
		}

		try {
			BufferedImage	img;

			img = ImageIO.read(pngfile);
			if (img == null) {
				System.out.printf("# Can't read pngfile %s\n", pngfile);
				return null;
			}
			return new ImageIcon(img);
		} catch (IOException e) {
			System.out.printf("# IO error trying to load %s\n", pngfile);
			return null;
		}
	}
}

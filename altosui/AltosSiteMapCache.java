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
import javax.imageio.ImageIO;
import javax.swing.table.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.net.URL;
import java.net.URLConnection;

public class AltosSiteMapCache extends JLabel {
	public static boolean fetchMap(File file, String url) {
		URL u;

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
			return new ImageIcon(ImageIO.read(pngfile));
		} catch (IOException e) {
			System.out.printf("# IO error trying to load %s\n", pngfile);
			return null;
		}
	}
}

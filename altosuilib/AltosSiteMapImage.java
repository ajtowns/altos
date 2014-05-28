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
import java.awt.image.*;
import javax.imageio.ImageIO;
import javax.swing.*;
import java.io.*;
import org.altusmetrum.altoslib_4.*;

public class AltosSiteMapImage {
	AltosSiteMapTile	tile;
	File			file;
	BufferedImage		image;
	int			width;
	int			height;
	long			used;

	Thread	load_thread;

	public boolean validate() {
		if (image != null) {
			AltosSiteMap.debug_component(tile, "valid");
			return true;
		} else {
			AltosSiteMap.debug_component(tile, "loading");
			load_thread = new Thread() {
					public void run() {
						image = null;
						try {
							image = ImageIO.read(file);
						} catch (Exception e) {
						}
						SwingUtilities.invokeLater( new Runnable() {
								public void run() {
									AltosSiteMap.debug_component(tile, "later");
									Graphics2D g2d = (Graphics2D) tile.getGraphics();
									tile.paint_graphics(g2d, image);
									load_thread = null;
								}
							});
					}
				};
			load_thread.start();
			return false;
		}
	}

	public void flush() {
		if (load_thread == null) {
			AltosSiteMap.debug_component(tile, "flush");
			image.flush();
			image = null;
		}
	}

	public AltosSiteMapImage (AltosSiteMapTile tile, File file, int w, int h) throws IOException {
		this.tile = tile;
		this.file = file;
		width = w;
		height = h;
		image = null;
		used = 0;
	}
}


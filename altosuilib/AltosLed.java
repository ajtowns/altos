/*
 * Copyright © 2010 Keith Packard <keithp@keithp.com>
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

public class AltosLed extends JLabel {
	ImageIcon	on, off;

	ImageIcon create_icon(String path) {
		java.net.URL imgURL = AltosUILib.class.getResource(path);
		if (imgURL != null)
			return new ImageIcon(imgURL);
		System.err.printf("Cannot find icon \"%s\"\n", path);
		return null;
	}

	public void set(boolean set) {
		if (set)
			setIcon(on);
		else
			setIcon(off);
	}

	public AltosLed(String on_path, String off_path) {
		on = create_icon(on_path);
		off = create_icon(off_path);
		setIcon(off);
	}
}

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

import java.awt.*;
import javax.swing.*;

public class AltosLights extends JComponent {

	GridBagLayout	gridbag;

	AltosLed	red, green;

	ImageIcon create_icon(String path, String description) {
		java.net.URL imgURL = AltosUILib.class.getResource(path);
		if (imgURL != null)
			return new ImageIcon(imgURL, description);
		System.err.printf("Cannot find icon \"%s\"\n", path);
		return null;
	}

	public void set (boolean on) {
		if (on) {
			red.set(false);
			green.set(true);
		} else {
			red.set(true);
			green.set(false);
		}
	}

	public AltosLights() {
		GridBagConstraints c;
		gridbag = new GridBagLayout();
		setLayout(gridbag);

		c = new GridBagConstraints();
		red = new AltosLed("/redled.png", "/grayled.png");
		c.gridx = 0; c.gridy = 0;
		c.insets = new Insets (0, 5, 0, 5);
		gridbag.setConstraints(red, c);
		add(red);
		red.set(true);
		green = new AltosLed("/greenled.png", "/grayled.png");
		c.gridx = 1; c.gridy = 0;
		gridbag.setConstraints(green, c);
		add(green);
		green.set(false);
	}
}

/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

package org.altusmetrum.micropeak;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.*;

class MicroFrameListener extends WindowAdapter {
	public void windowClosing (WindowEvent e) {
		MicroPreferences.unregister_ui_listener((MicroFrame) e.getWindow());
	}
}

public class MicroFrame extends JFrame implements MicroUIListener {

	public void ui_changed(String look_and_feel) {
		SwingUtilities.updateComponentTreeUI(this);
		this.pack();
	}

	static final String[] icon_names = {
		"/micropeak-16.png",
		"/micropeak-32.png",
		"/micropeak-48.png",
		"/micropeak-64.png",
		"/micropeak-128.png",
		"/micropeak-256.png"
	};

	public void set_icon() {
		ArrayList<Image> icons = new ArrayList<Image>();
		
		for (int i = 0; i < icon_names.length; i++) {
			java.net.URL imgURL = MicroPeak.class.getResource(icon_names[i]);
			if (imgURL != null)
				icons.add(new ImageIcon(imgURL).getImage());
		}

		setIconImages(icons);
	}
			
	public MicroFrame() {
		super();
		MicroPreferences.set_component(this);
		MicroPreferences.register_ui_listener(this);
		addWindowListener(new MicroFrameListener());
		set_icon();
	}

	public MicroFrame(String name) {
		super(name);
		MicroPreferences.set_component(this);
		MicroPreferences.register_ui_listener(this);
		addWindowListener(new MicroFrameListener());
		set_icon();
	}
}

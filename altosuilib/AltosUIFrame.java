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

package org.altusmetrum.altosuilib;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.*;

class AltosUIFrameListener extends WindowAdapter {
	public void windowClosing (WindowEvent e) {
		AltosUIPreferences.unregister_ui_listener((AltosUIFrame) e.getWindow());
	}
}

public class AltosUIFrame extends JFrame implements AltosUIListener {

	public void ui_changed(String look_and_feel) {
		SwingUtilities.updateComponentTreeUI(this);
		this.pack();
	}

	static String[] altos_icon_names = {
		"/altus-metrum-16.png",
		"/altus-metrum-32.png",
		"/altus-metrum-48.png",
		"/altus-metrum-64.png",
		"/altus-metrum-128.png",
		"/altus-metrum-256.png"
	};

	static public String[] icon_names;
	
	static public void set_icon_names(String[] new_icon_names) { icon_names = new_icon_names; }

	public String[] icon_names() {
		if (icon_names == null)
			set_icon_names(altos_icon_names);
		return icon_names;
	}

	public void set_icon() {
		ArrayList<Image> icons = new ArrayList<Image>();
		String[] icon_names = icon_names();
		
		for (int i = 0; i < icon_names.length; i++) {
			java.net.URL imgURL = AltosUIFrame.class.getResource(icon_names[i]);
			if (imgURL != null)
				icons.add(new ImageIcon(imgURL).getImage());
		}
		setIconImages(icons);
	}
			
	private boolean location_by_platform = true;

	public void setLocationByPlatform(boolean lbp) {
		location_by_platform = lbp;
		super.setLocationByPlatform(lbp);
	}
		
	public void setVisible (boolean visible) {
		if (visible)
			setLocationByPlatform(location_by_platform);
		super.setVisible(visible);
	}
		
	public AltosUIFrame() {
		AltosUIPreferences.register_ui_listener(this);
		addWindowListener(new AltosUIFrameListener());
		set_icon();
	}

	public AltosUIFrame(String name) {
		super(name);
		AltosUIPreferences.register_ui_listener(this);
		addWindowListener(new AltosUIFrameListener());
		set_icon();
	}
}

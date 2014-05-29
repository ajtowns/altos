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

package org.altusmetrum.altosuilib_2;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.*;

class AltosUIFrameListener extends WindowAdapter {
	public void windowClosing (WindowEvent e) {
		AltosUIPreferences.unregister_ui_listener((AltosUIFrame) e.getWindow());
	}
}

public class AltosUIFrame extends JFrame implements AltosUIListener, AltosPositionListener {

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

	public void scan_device_selected(AltosDevice device) {
	}

	public void setSize() {
		/* Smash sizes around so that the window comes up in the right shape */
		Insets i = getInsets();
		Dimension ps = rootPane.getPreferredSize();
		ps.width += i.left + i.right;
		ps.height += i.top + i.bottom;
		setPreferredSize(ps);
		setSize(ps);
	}

	public void setPosition (int position) {
		Insets i = getInsets();
		Dimension ps = getSize();

		/* Stick the window in the desired location on the screen */
		setLocationByPlatform(false);
		GraphicsDevice gd = GraphicsEnvironment.getLocalGraphicsEnvironment().getDefaultScreenDevice();
		GraphicsConfiguration gc = gd.getDefaultConfiguration();
		Rectangle r = gc.getBounds();

		/* compute X position */
		int x = 0;
		int y = 0;
		switch (position) {
		case AltosUILib.position_top_left:
		case AltosUILib.position_left:
		case AltosUILib.position_bottom_left:
			x = 0;
			break;
		case AltosUILib.position_top:
		case AltosUILib.position_center:
		case AltosUILib.position_bottom:
			x = (r.width - ps.width) / 2;
			break;
		case AltosUILib.position_top_right:
		case AltosUILib.position_right:
		case AltosUILib.position_bottom_right:
			x = r.width - ps.width + i.right;
			break;
		}

		/* compute Y position */
		switch (position) {
		case AltosUILib.position_top_left:
		case AltosUILib.position_top:
		case AltosUILib.position_top_right:
			y = 0;
			break;
		case AltosUILib.position_left:
		case AltosUILib.position_center:
		case AltosUILib.position_right:
			y = (r.height - ps.height) / 2;
			break;
		case AltosUILib.position_bottom_left:
		case AltosUILib.position_bottom:
		case AltosUILib.position_bottom_right:
			y = r.height - ps.height + i.bottom;
			break;
		}
		setLocation(x, y);
	}

	int position;

	public void position_changed(int position) {
		this.position = position;
		if (!location_by_platform)
			setPosition(position);
	}

	public void setVisible (boolean visible) {
		if (visible)
			setLocationByPlatform(location_by_platform);
		super.setVisible(visible);
		if (visible) {
			setSize();
			if (!location_by_platform)
				setPosition(position);
		}
	}

	void init() {
		AltosUIPreferences.register_ui_listener(this);
		AltosUIPreferences.register_position_listener(this);
		position = AltosUIPreferences.position();
		addWindowListener(new AltosUIFrameListener());
		set_icon();
	}

	public AltosUIFrame() {
		init();
	}

	public AltosUIFrame(String name) {
		super(name);
		init();
	}
}

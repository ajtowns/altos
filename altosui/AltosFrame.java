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

package altosui;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.*;

class AltosFrameListener extends WindowAdapter {
	public void windowClosing (WindowEvent e) {
		AltosUIPreferences.unregister_ui_listener((AltosFrame) e.getWindow());
	}
}

public class AltosFrame extends JFrame implements AltosUIListener {

	public void ui_changed(String look_and_feel) {
		SwingUtilities.updateComponentTreeUI(this);
		this.pack();
	}

	static final String[] icon_names = {
		"/altus-metrum-16.png",
		"/altus-metrum-32.png",
		"/altus-metrum-48.png",
		"/altus-metrum-64.png",
		"/altus-metrum-128.png",
		"/altus-metrum-256.png"
	};

	public void set_icon() {
		ArrayList<Image> icons = new ArrayList<Image>();
		
		for (int i = 0; i < icon_names.length; i++) {
			java.net.URL imgURL = AltosUI.class.getResource(icon_names[i]);
			if (imgURL != null)
				icons.add(new ImageIcon(imgURL).getImage());
		}

		setIconImages(icons);
	}
			
	public AltosFrame() {
		AltosUIPreferences.register_ui_listener(this);
		addWindowListener(new AltosFrameListener());
		set_icon();
	}

	public AltosFrame(String name) {
		super(name);
		AltosUIPreferences.register_ui_listener(this);
		addWindowListener(new AltosFrameListener());
		set_icon();
	}
}

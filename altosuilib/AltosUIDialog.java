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

class AltosUIDialogListener extends WindowAdapter {
	public void windowClosing (WindowEvent e) {
		AltosUIPreferences.unregister_ui_listener((AltosUIDialog) e.getWindow());
	}
}

public class AltosUIDialog extends JDialog implements AltosUIListener {

	public void ui_changed(String look_and_feel) {
		SwingUtilities.updateComponentTreeUI(this);
		this.pack();
	}

	public AltosUIDialog() {
		AltosUIPreferences.register_ui_listener(this);
		addWindowListener(new AltosUIDialogListener());
	}

	public AltosUIDialog(Frame frame, String label, boolean modal) {
		super(frame, label, modal);
		AltosUIPreferences.register_ui_listener(this);
		addWindowListener(new AltosUIDialogListener());
	}

	public AltosUIDialog(Dialog dialog, String label, boolean modal) {
		super(dialog, label, modal);
		AltosUIPreferences.register_ui_listener(this);
		addWindowListener(new AltosUIDialogListener());
	}

	public AltosUIDialog(Frame frame, boolean modal) {
		super(frame, modal);
		AltosUIPreferences.register_ui_listener(this);
		addWindowListener(new AltosUIDialogListener());
	}
}

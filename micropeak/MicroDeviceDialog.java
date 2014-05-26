/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import org.altusmetrum.altosuilib_2.*;

public class MicroDeviceDialog extends AltosDeviceDialog {

	public AltosDevice[] devices() {
		java.util.List<MicroUSB>	list = MicroUSB.list();

		if (list == null) {
			JOptionPane.showMessageDialog(frame,
						      "libaltos failed to load",
						      "Helper Library Failed",
						      JOptionPane.ERROR_MESSAGE);
			return new AltosDevice[0];
		}

		int		num_devices = list.size();
		AltosDevice[]	devices = new AltosDevice[num_devices];

		for (int i = 0; i < num_devices; i++)
			devices[i] = list.get(i);
		return devices;
	}

	public MicroDeviceDialog (Frame in_frame, Component location) {
		super(in_frame, location, 0);
	}

	public static AltosDevice show (Component frameComp) {
		Frame			frame = JOptionPane.getFrameForComponent(frameComp);
		MicroDeviceDialog	dialog;

		dialog = new MicroDeviceDialog (frame, frameComp);
		dialog.setVisible(true);
		return dialog.getValue();
	}
}

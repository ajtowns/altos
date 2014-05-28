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
import java.awt.*;
import java.awt.event.*;

public class AltosDeviceUIDialog extends AltosDeviceDialog {

	public AltosDevice[] devices() {
		java.util.List<AltosDevice>	usb_devices = AltosUSBDevice.list(product);
		int				num_devices = usb_devices.size();
		java.util.List<AltosDevice>	bt_devices = AltosBTKnown.bt_known().list(product);
		num_devices += bt_devices.size();
		AltosDevice[]			devices = new AltosDevice[num_devices];

		for (int i = 0; i < usb_devices.size(); i++)
			devices[i] = usb_devices.get(i);
		int off = usb_devices.size();
		for (int j = 0; j < bt_devices.size(); j++)
			devices[off + j] = bt_devices.get(j);
		return devices;
	}

	public void add_bluetooth() {
		JButton manage_bluetooth_button = new JButton("Manage Bluetooth");
		manage_bluetooth_button.setActionCommand("manage");
		manage_bluetooth_button.addActionListener(this);
		buttonPane.add(manage_bluetooth_button);
		buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));
	}

	public void actionPerformed(ActionEvent e) {
		super.actionPerformed(e);
		if ("manage".equals(e.getActionCommand())) {
			AltosBTManage.show(frame, AltosBTKnown.bt_known());
			update_devices();
		}
	}

	public AltosDeviceUIDialog (Frame in_frame, Component location, int in_product) {
		super(in_frame, location, in_product);
	}

	public static AltosDevice show (Component frameComp, int product) {
		Frame			frame = JOptionPane.getFrameForComponent(frameComp);
		AltosDeviceUIDialog	dialog;

		dialog = new AltosDeviceUIDialog(frame, frameComp, product);
		dialog.setVisible(true);
		return dialog.getValue();
	}
}

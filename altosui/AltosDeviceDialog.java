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

package altosui;

import java.lang.*;
import java.util.*;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import libaltosJNI.*;

public class AltosDeviceDialog extends AltosDialog implements ActionListener {

	private AltosDevice	value;
	private JList		list;
	private JButton		cancel_button;
	private JButton		select_button;
	private JButton		manage_bluetooth_button;
	private Frame		frame;
	private int		product;

	private AltosDevice getValue() {
		return value;
	}

	private AltosDevice[] devices() {
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

	private void update_devices() {
		AltosDevice[] devices = devices();
		list.setListData(devices);
		select_button.setEnabled(devices.length > 0);
	}

	private AltosDeviceDialog (Frame in_frame, Component location, int in_product) {
		super(in_frame, "Device Selection", true);

		product = in_product;
		frame = in_frame;
		value = null;

		AltosDevice[]	devices = devices();

		cancel_button = new JButton("Cancel");
		cancel_button.setActionCommand("cancel");
		cancel_button.addActionListener(this);

		manage_bluetooth_button = new JButton("Manage Bluetooth");
		manage_bluetooth_button.setActionCommand("manage");
		manage_bluetooth_button.addActionListener(this);

		select_button = new JButton("Select");
		select_button.setActionCommand("select");
		select_button.addActionListener(this);
		if (devices.length == 0)
			select_button.setEnabled(false);
		getRootPane().setDefaultButton(select_button);

		list = new JList(devices) {
				//Subclass JList to workaround bug 4832765, which can cause the
				//scroll pane to not let the user easily scroll up to the beginning
				//of the list.  An alternative would be to set the unitIncrement
				//of the JScrollBar to a fixed value. You wouldn't get the nice
				//aligned scrolling, but it should work.
				public int getScrollableUnitIncrement(Rectangle visibleRect,
								      int orientation,
								      int direction) {
					int row;
					if (orientation == SwingConstants.VERTICAL &&
					    direction < 0 && (row = getFirstVisibleIndex()) != -1) {
						Rectangle r = getCellBounds(row, row);
						if ((r.y == visibleRect.y) && (row != 0))  {
							Point loc = r.getLocation();
							loc.y--;
							int prevIndex = locationToIndex(loc);
							Rectangle prevR = getCellBounds(prevIndex, prevIndex);

							if (prevR == null || prevR.y >= r.y) {
								return 0;
							}
							return prevR.height;
						}
					}
					return super.getScrollableUnitIncrement(
						visibleRect, orientation, direction);
				}
			};

		list.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		list.setLayoutOrientation(JList.HORIZONTAL_WRAP);
		list.setVisibleRowCount(-1);
		list.addMouseListener(new MouseAdapter() {
				 public void mouseClicked(MouseEvent e) {
					 if (e.getClickCount() == 2) {
						 select_button.doClick(); //emulate button click
					 }
				 }
			});
		JScrollPane listScroller = new JScrollPane(list);
		listScroller.setPreferredSize(new Dimension(400, 80));
		listScroller.setAlignmentX(LEFT_ALIGNMENT);

		//Create a container so that we can add a title around
		//the scroll pane.  Can't add a title directly to the
		//scroll pane because its background would be white.
		//Lay out the label and scroll pane from top to bottom.
		JPanel listPane = new JPanel();
		listPane.setLayout(new BoxLayout(listPane, BoxLayout.PAGE_AXIS));

		JLabel label = new JLabel("Select Device");
		label.setLabelFor(list);
		listPane.add(label);
		listPane.add(Box.createRigidArea(new Dimension(0,5)));
		listPane.add(listScroller);
		listPane.setBorder(BorderFactory.createEmptyBorder(10,10,10,10));

		//Lay out the buttons from left to right.
		JPanel buttonPane = new JPanel();
		buttonPane.setLayout(new BoxLayout(buttonPane, BoxLayout.LINE_AXIS));
		buttonPane.setBorder(BorderFactory.createEmptyBorder(0, 10, 10, 10));
		buttonPane.add(Box.createHorizontalGlue());
		buttonPane.add(cancel_button);
		buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));
		buttonPane.add(manage_bluetooth_button);
		buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));
		buttonPane.add(select_button);

		//Put everything together, using the content pane's BorderLayout.
		Container contentPane = getContentPane();
		contentPane.add(listPane, BorderLayout.CENTER);
		contentPane.add(buttonPane, BorderLayout.PAGE_END);

		//Initialize values.
		if (devices != null && devices.length != 0)
			list.setSelectedValue(devices[0], true);
		pack();
		setLocationRelativeTo(location);
	}

	//Handle clicks on the Set and Cancel buttons.
	public void actionPerformed(ActionEvent e) {
		if ("select".equals(e.getActionCommand()))
			value = (AltosDevice)(list.getSelectedValue());
		if ("manage".equals(e.getActionCommand())) {
			AltosBTManage.show(frame, AltosBTKnown.bt_known());
			update_devices();
			return;
		}
		setVisible(false);
	}

	public static AltosDevice show (Component frameComp, int product) {

		Frame				frame = JOptionPane.getFrameForComponent(frameComp);
		AltosDeviceDialog	dialog;

		dialog = new AltosDeviceDialog(frame, frameComp, product);
		dialog.setVisible(true);
		return dialog.getValue();
	}
}

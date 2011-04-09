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
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.table.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.util.concurrent.*;

import libaltosJNI.*;

public class AltosBTManage extends JDialog implements ActionListener {
	String	product;
	LinkedBlockingQueue<AltosBTDevice> found_devices;
	JFrame frame;

	class DeviceList extends JList implements Iterable<AltosBTDevice> {
		LinkedList<AltosBTDevice> devices;
		DefaultListModel	list_model;

		public void add (AltosBTDevice device) {
			devices.add(device);
			list_model.addElement(device);
		}

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

		public Iterator<AltosBTDevice> iterator() {
			return devices.iterator();
		}

		public DeviceList() {
			devices = new LinkedList<AltosBTDevice>();
			list_model = new DefaultListModel();
			setModel(list_model);
			setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
			setLayoutOrientation(JList.HORIZONTAL_WRAP);
			setVisibleRowCount(-1);
		}
	}

	DeviceList	visible_devices;

	DeviceList	selected_devices;

	public void actionPerformed(ActionEvent e) {
	}

	public void got_visible_device() {
		while (!found_devices.isEmpty()) {
			AltosBTDevice	device = found_devices.remove();
			visible_devices.add(device);
		}
	}

	class BTGetVisibleDevices implements Runnable {
		public void run () {

			try {
				AltosBTDeviceIterator	i = new AltosBTDeviceIterator(product);
				AltosBTDevice		device;

				while ((device = i.next()) != null) {
					Runnable r;

					found_devices.add(device);
					r = new Runnable() {
							public void run() {
								got_visible_device();
							}
						};
					SwingUtilities.invokeLater(r);
				}
			} catch (Exception e) {
				System.out.printf("uh-oh, exception %s\n", e.toString());
			}
		}
	}

	public AltosBTManage(String product, JFrame in_frame) {
		frame = in_frame;
		BTGetVisibleDevices	get_visible_devices = new BTGetVisibleDevices();
		Thread t = new Thread(get_visible_devices);
		t.start();

		found_devices = new LinkedBlockingQueue<AltosBTDevice>();

		JButton cancelButton = new JButton("Cancel");
		cancelButton.addActionListener(this);

		final JButton selectButton = new JButton("Select");
		selectButton.setActionCommand("select");
		selectButton.addActionListener(this);
		getRootPane().setDefaultButton(selectButton);

		selected_devices = new DeviceList();
		JScrollPane selected_list_scroller = new JScrollPane(selected_devices);
		selected_list_scroller.setPreferredSize(new Dimension(400, 80));
		selected_list_scroller.setAlignmentX(LEFT_ALIGNMENT);

		visible_devices = new DeviceList();
		JScrollPane visible_list_scroller = new JScrollPane(visible_devices);
		visible_list_scroller.setPreferredSize(new Dimension(400, 80));
		visible_list_scroller.setAlignmentX(LEFT_ALIGNMENT);

		//Create a container so that we can add a title around
		//the scroll pane.  Can't add a title directly to the
		//scroll pane because its background would be white.
		//Lay out the label and scroll pane from top to bottom.
		JPanel listPane = new JPanel();
		listPane.setLayout(new BoxLayout(listPane, BoxLayout.PAGE_AXIS));

		JLabel label = new JLabel("Select Device");
		label.setLabelFor(selected_devices);
		listPane.add(label);
		listPane.add(Box.createRigidArea(new Dimension(0,5)));
		listPane.add(selected_list_scroller);
		listPane.add(visible_list_scroller);
		listPane.setBorder(BorderFactory.createEmptyBorder(10,10,10,10));

		//Lay out the buttons from left to right.
		JPanel buttonPane = new JPanel();
		buttonPane.setLayout(new BoxLayout(buttonPane, BoxLayout.LINE_AXIS));
		buttonPane.setBorder(BorderFactory.createEmptyBorder(0, 10, 10, 10));
		buttonPane.add(Box.createHorizontalGlue());
		buttonPane.add(cancelButton);
		buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));
		buttonPane.add(selectButton);

		//Put everything together, using the content pane's BorderLayout.
		Container contentPane = getContentPane();
		contentPane.add(listPane, BorderLayout.CENTER);
		contentPane.add(buttonPane, BorderLayout.PAGE_END);

		//Initialize values.
//		list.setSelectedValue(initial, true);
		pack();
		setLocationRelativeTo(frame);
		setVisible(true);
	}
}

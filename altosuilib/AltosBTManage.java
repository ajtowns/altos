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
import javax.swing.plaf.basic.*;
import java.util.*;
import java.util.concurrent.*;
import org.altusmetrum.altoslib_4.*;

public class AltosBTManage extends AltosUIDialog implements ActionListener, Iterable<AltosBTDevice> {
	LinkedBlockingQueue<AltosBTDevice> found_devices;
	Frame frame;
	LinkedList<ActionListener> listeners;
	AltosBTKnown	bt_known;

	class DeviceList extends JList<AltosBTDevice> implements Iterable<AltosBTDevice> {
		LinkedList<AltosBTDevice> 	devices;
		DefaultListModel<AltosBTDevice>	list_model;

		public void add (AltosBTDevice device) {
			if (!devices.contains(device)) {
				devices.add(device);
				list_model.addElement(device);
			}
		}

		public void remove (AltosBTDevice device) {
			if (devices.contains(device)) {
				devices.remove(device);
				list_model.removeElement(device);
			}
		}

		public boolean contains(AltosBTDevice device) {
			return devices.contains(device);
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

		public java.util.List<AltosBTDevice> selected_list() throws InterruptedException {
			return getSelectedValuesList();
		}

		public DeviceList() {
			devices = new LinkedList<AltosBTDevice>();
			list_model = new DefaultListModel<AltosBTDevice>();
			setModel(list_model);
			setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
			setLayoutOrientation(JList.HORIZONTAL_WRAP);
			setVisibleRowCount(-1);
		}
	}

	DeviceList	visible_devices;

	DeviceList	known_devices;
	Thread		bt_thread;

	public Iterator<AltosBTDevice> iterator() {
		return known_devices.iterator();
	}

	public void commit() {
		bt_known.set(this);
	}

	public void add_known() {
		try {
			for (AltosBTDevice device : visible_devices.selected_list()) {
				known_devices.add(device);
				visible_devices.remove(device);
			}
		} catch (InterruptedException ie) {
		}
	}

	public void remove_known() {
		try {
			for (AltosBTDevice device : known_devices.selected_list()) {
				known_devices.remove(device);
				visible_devices.add(device);
			}
		} catch (InterruptedException ie) {
		}
	}

	public void addActionListener(ActionListener l) {
		listeners.add(l);
	}

	private void forwardAction(ActionEvent e) {
		for (ActionListener l : listeners)
			l.actionPerformed(e);
	}

	public void actionPerformed(ActionEvent e) {
		String	command = e.getActionCommand();
		if ("ok".equals(command)) {
			bt_thread.interrupt();
			commit();
			setVisible(false);
			forwardAction(e);
		} else if ("cancel".equals(command)) {
			bt_thread.interrupt();
			setVisible(false);
			forwardAction(e);
		} else if ("select".equals(command)) {
			add_known();
		} else if ("deselect".equals(command)) {
			remove_known();
		}
	}

	public void got_visible_device() {
		while (!found_devices.isEmpty()) {
			AltosBTDevice	device = found_devices.remove();
			if (!known_devices.contains(device))
				visible_devices.add(device);
		}
	}

	class BTGetVisibleDevices implements Runnable {
		public void run () {
			for (;;)
				for (int time = 1; time <= 8; time <<= 1) {
					AltosBTDeviceIterator	i = new AltosBTDeviceIterator(time);
					AltosBTDevice		device;

					if (Thread.interrupted())
						return;
					try {
						while ((device = i.next()) != null) {
							Runnable r;

							if (Thread.interrupted())
								return;
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
	}

	public static void show(Component frameComp, AltosBTKnown known) {
		Frame	frame = JOptionPane.getFrameForComponent(frameComp);
		AltosBTManage	dialog;

		dialog = new AltosBTManage(frame, known);
		dialog.setVisible(true);
	}

	public AltosBTManage(Frame in_frame, AltosBTKnown in_known) {
		super(in_frame, "Manage Bluetooth Devices", true);

		frame = in_frame;
		bt_known = in_known;
		BTGetVisibleDevices	get_visible_devices = new BTGetVisibleDevices();
		bt_thread = new Thread(get_visible_devices);
		bt_thread.start();

		listeners = new LinkedList<ActionListener>();

		found_devices = new LinkedBlockingQueue<AltosBTDevice>();

		Container pane = getContentPane();
		pane.setLayout(new GridBagLayout());

		GridBagConstraints c = new GridBagConstraints();
		c.insets = new Insets(4,4,4,4);

		/*
		 * Known devices label and list
		 */
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		c.gridx = 0;
		c.gridy = 0;
		c.gridwidth = 1;
		c.gridheight = 1;
		c.weightx = 0;
		c.weighty = 0;
		pane.add(new JLabel("Known Devices"), c);

		known_devices = new DeviceList();
		for (AltosBTDevice device : bt_known)
			known_devices.add(device);

		JScrollPane known_list_scroller = new JScrollPane(known_devices);
		known_list_scroller.setPreferredSize(new Dimension(400, 80));
		known_list_scroller.setAlignmentX(LEFT_ALIGNMENT);
		c.fill = GridBagConstraints.BOTH;
		c.anchor = GridBagConstraints.WEST;
		c.gridx = 0;
		c.gridy = 1;
		c.gridwidth = 1;
		c.gridheight = 2;
		c.weightx = 1;
		c.weighty = 1;
		pane.add(known_list_scroller, c);

		/*
		 * Visible devices label and list
		 */
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		c.gridx = 2;
		c.gridy = 0;
		c.gridwidth = 1;
		c.gridheight = 1;
		c.weightx = 0;
		c.weighty = 0;

		pane.add(new JLabel("Visible Devices"), c);

		visible_devices = new DeviceList();
		JScrollPane visible_list_scroller = new JScrollPane(visible_devices);
		visible_list_scroller.setPreferredSize(new Dimension(400, 80));
		visible_list_scroller.setAlignmentX(LEFT_ALIGNMENT);
		c.fill = GridBagConstraints.BOTH;
		c.anchor = GridBagConstraints.WEST;
		c.gridx = 2;
		c.gridy = 1;
		c.gridheight = 2;
		c.gridwidth = 1;
		c.weightx = 1;
		c.weighty = 1;
		pane.add(visible_list_scroller, c);

		/*
		 * Arrows between the two lists
		 */
		BasicArrowButton select_arrow = new BasicArrowButton(SwingConstants.WEST);
		select_arrow.setActionCommand("select");
		select_arrow.addActionListener(this);
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.SOUTH;
		c.gridx = 1;
		c.gridy = 1;
		c.gridheight = 1;
		c.gridwidth = 1;
		c.weightx = 0;
		c.weighty = 0;
		pane.add(select_arrow, c);

		BasicArrowButton deselect_arrow = new BasicArrowButton(SwingConstants.EAST);
		deselect_arrow.setActionCommand("deselect");
		deselect_arrow.addActionListener(this);
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.NORTH;
		c.gridx = 1;
		c.gridy = 2;
		c.gridheight = 1;
		c.gridwidth = 1;
		c.weightx = 0;
		c.weighty = 0;
		pane.add(deselect_arrow, c);

		JButton cancel_button = new JButton("Cancel");
		cancel_button.setActionCommand("cancel");
		cancel_button.addActionListener(this);
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.gridx = 0;
		c.gridy = 3;
		c.gridheight = 1;
		c.gridwidth = 1;
		c.weightx = 0;
		c.weighty = 0;
		pane.add(cancel_button, c);

		JButton ok_button = new JButton("OK");
		ok_button.setActionCommand("ok");
		ok_button.addActionListener(this);
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.gridx = 2;
		c.gridy = 3;
		c.gridheight = 1;
		c.gridwidth = 1;
		c.weightx = 0;
		c.weighty = 0;
		pane.add(ok_button, c);

		getRootPane().setDefaultButton(ok_button);

		pack();
		setLocationRelativeTo(frame);
		setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
		addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosing(WindowEvent e) {
				bt_thread.interrupt();
				setVisible(false);
			}
		});
	}
}

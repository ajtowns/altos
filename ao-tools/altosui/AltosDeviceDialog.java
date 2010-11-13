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
import libaltosJNI.libaltos;
import libaltosJNI.altos_device;
import libaltosJNI.SWIGTYPE_p_altos_file;
import libaltosJNI.SWIGTYPE_p_altos_list;

public class AltosDeviceDialog extends JDialog implements ActionListener {

	private static AltosDeviceDialog	dialog;
	private static AltosDevice		value = null;
	private JList				list;

	public static AltosDevice show (Component frameComp, int product) {

		Frame frame = JOptionPane.getFrameForComponent(frameComp);
		AltosDevice[]	devices;
		devices = AltosDevice.list(product);

		if (devices != null && devices.length > 0) {
			value = null;
			dialog = new AltosDeviceDialog(frame, frameComp,
						       devices,
						       devices[0]);

			dialog.setVisible(true);
			return value;
		} else {
			/* check for missing altos JNI library, which
			 * will put up its own error dialog
			 */
			if (AltosUI.load_library(frame)) {
				JOptionPane.showMessageDialog(frame,
							      "No AltOS devices available",
							      "No AltOS devices",
							      JOptionPane.ERROR_MESSAGE);
			}
			return null;
		}
	}

	private AltosDeviceDialog (Frame frame, Component location,
				   AltosDevice[] devices,
				   AltosDevice initial) {
		super(frame, "Device Selection", true);

		value = null;

		JButton cancelButton = new JButton("Cancel");
		cancelButton.addActionListener(this);

		final JButton selectButton = new JButton("Select");
		selectButton.setActionCommand("select");
		selectButton.addActionListener(this);
		getRootPane().setDefaultButton(selectButton);

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
						 selectButton.doClick(); //emulate button click
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
		buttonPane.add(cancelButton);
		buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));
		buttonPane.add(selectButton);

		//Put everything together, using the content pane's BorderLayout.
		Container contentPane = getContentPane();
		contentPane.add(listPane, BorderLayout.CENTER);
		contentPane.add(buttonPane, BorderLayout.PAGE_END);

		//Initialize values.
		list.setSelectedValue(initial, true);
		pack();
		setLocationRelativeTo(location);
	}

	//Handle clicks on the Set and Cancel buttons.
	public void actionPerformed(ActionEvent e) {
		if ("select".equals(e.getActionCommand()))
			AltosDeviceDialog.value = (AltosDevice)(list.getSelectedValue());
		AltosDeviceDialog.dialog.setVisible(false);
	}

}

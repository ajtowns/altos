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
import org.altusmetrum.altoslib_4.*;

class AltosEditFreqUI extends AltosUIDialog implements ActionListener {
	Frame		frame;
	JTextField	frequency;
	JTextField	description;
	JButton		ok_button, cancel_button;
	boolean		got_ok;

	public void actionPerformed(ActionEvent e) {
		String	cmd = e.getActionCommand();

		if ("ok".equals(cmd)) {
			got_ok = true;
			setVisible(false);
		}
		if ("cancel".equals(cmd)) {
			got_ok = false;
			setVisible(false);
		}
	}

	public AltosFrequency get() {
		if (!got_ok)
			return null;

		String	f_s = frequency.getText();
		String	d_s = description.getText();

		try {
			double	f_d = Double.parseDouble(f_s);

			return new AltosFrequency(f_d, d_s);
		} catch (NumberFormatException ne) {
		}
		return null;
	}

	public AltosEditFreqUI(Frame in_frame, AltosFrequency existing) {
		super(in_frame, true);

		got_ok = false;
		frame = in_frame;

		Container pane = getContentPane();
		pane.setLayout(new GridBagLayout());

		GridBagConstraints c = new GridBagConstraints();
		c.insets = new Insets (4,4,4,4);

		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		c.gridx = 0;
		c.gridy = 0;
		c.gridwidth = 1;
		c.gridheight = 1;
		c.weightx = 0;
		c.weighty = 0;
		pane.add(new JLabel("Frequency"), c);

		frequency = new JTextField(12);
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		c.gridx = 1;
		c.gridy = 0;
		c.gridwidth = 1;
		c.gridheight = 1;
		c.weightx = 0;
		c.weighty = 0;
		pane.add(frequency, c);

		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		c.gridx = 0;
		c.gridy = 1;
		c.gridwidth = 1;
		c.gridheight = 1;
		c.weightx = 0;
		c.weighty = 0;
		pane.add(new JLabel("Description"), c);

		description = new JTextField(12);
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		c.gridx = 1;
		c.gridy = 1;
		c.gridwidth = 1;
		c.gridheight = 1;
		c.weightx = 0;
		c.weighty = 0;
		pane.add(description, c);

		ok_button = new JButton("OK");
		ok_button.setActionCommand("ok");
		ok_button.addActionListener(this);
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		c.gridx = 0;
		c.gridy = 2;
		c.gridwidth = 1;
		c.gridheight = 1;
		c.weightx = 0;
		c.weighty = 0;
		pane.add(ok_button, c);

		cancel_button = new JButton("Cancel");
		cancel_button.setActionCommand("cancel");
		cancel_button.addActionListener(this);
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		c.gridx = 1;
		c.gridy = 2;
		c.gridwidth = 1;
		c.gridheight = 1;
		c.weightx = 0;
		c.weighty = 0;
		pane.add(cancel_button, c);

		if (existing == null)
			setTitle("Add New Frequency");
		else {
			setTitle("Edit Existing Frequency");
			frequency.setText(String.format("%7.3f", existing.frequency));
			description.setText(existing.description);
		}
		getRootPane().setDefaultButton(ok_button);

		pack();
		setLocationRelativeTo(frame);

	}

	public AltosEditFreqUI(Frame in_frame) {
		this(in_frame, (AltosFrequency) null);
	}
}

public class AltosConfigFreqUI extends AltosUIDialog implements ActionListener {

	Frame frame;
	LinkedList<ActionListener> listeners;

	class FrequencyList extends JList<AltosFrequency> {
		DefaultListModel<AltosFrequency> list_model;

		public void add(AltosFrequency frequency) {
			int i;
			for (i = 0; i < list_model.size(); i++) {
				AltosFrequency	f = (AltosFrequency) list_model.get(i);
				if (frequency.frequency == f.frequency)
					return;
				if (frequency.frequency < f.frequency)
					break;
			}
			list_model.insertElementAt(frequency, i);
		}

		public void remove(AltosFrequency frequency) {
			list_model.removeElement(frequency);
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

		public AltosFrequency selected() {
			AltosFrequency	f = (AltosFrequency) getSelectedValue();
			return f;
		}

		public AltosFrequency[] frequencies() {
			AltosFrequency[]	ret;

			ret = new AltosFrequency[list_model.size()];
			for (int i = 0; i < list_model.size(); i++)
				ret[i] = (AltosFrequency) list_model.get(i);
			return ret;
		}

		public FrequencyList(AltosFrequency[] in_frequencies) {
			list_model = new DefaultListModel<AltosFrequency>();
			setModel(list_model);
			setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
			setLayoutOrientation(JList.HORIZONTAL_WRAP);
			for (int i = 0; i < in_frequencies.length; i++) {
				add(in_frequencies[i]);
			}
			setVisibleRowCount(in_frequencies.length);
		}
	}

	FrequencyList	frequencies;

	void save_frequencies() {
		AltosUIPreferences.set_common_frequencies(frequencies.frequencies());
	}

	JButton	add, edit, remove;

	JButton cancel, ok;

	public void actionPerformed(ActionEvent e) {
		String	cmd = e.getActionCommand();

		if ("ok".equals(cmd)) {
			save_frequencies();
			setVisible(false);
		} else if ("cancel".equals(cmd)) {
			setVisible(false);
		} else if ("add".equals(cmd)) {
			AltosEditFreqUI	ui = new AltosEditFreqUI(frame);
			ui.setVisible(true);
			AltosFrequency	f = ui.get();
			if (f != null)
				frequencies.add(f);
		} else if ("edit".equals(cmd)) {
			AltosFrequency	old_f = frequencies.selected();
			if (old_f == null)
				return;
			AltosEditFreqUI	ui = new AltosEditFreqUI(frame, old_f);
			ui.setVisible(true);
			AltosFrequency	new_f = ui.get();
			if (new_f != null) {
				if (old_f != null)
					frequencies.remove(old_f);
				frequencies.add(new_f);
			}
		} else if ("remove".equals(cmd)) {
			AltosFrequency	old_f = frequencies.selected();
			if (old_f == null)
				return;
			int ret = JOptionPane.showConfirmDialog(this,
								String.format("Remove frequency \"%s\"?",
									      old_f.toShortString()),
								"Remove Frequency",
								JOptionPane.YES_NO_OPTION);
			if (ret == JOptionPane.YES_OPTION)
				frequencies.remove(old_f);
		}
	}

	public AltosFrequency[] frequencies() {
		return frequencies.frequencies();
	}

	public AltosConfigFreqUI(Frame in_frame,
				 AltosFrequency[] in_frequencies) {
		super(in_frame, "Manage Frequencies", true);

		frame = in_frame;

		listeners = new LinkedList<ActionListener>();

		frequencies = new FrequencyList(in_frequencies);

		Container pane = getContentPane();
		pane.setLayout(new GridBagLayout());

		GridBagConstraints c = new GridBagConstraints();
		c.insets = new Insets(4,4,4,4);

		/*
		 * Frequencies label and list
		 */
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		c.gridx = 0;
		c.gridy = 0;
		c.gridwidth = 1;
		c.gridheight = 1;
		c.weightx = 0;
		c.weighty = 0;
		pane.add(new JLabel("Frequencies"), c);

		JScrollPane list_scroller = new JScrollPane(frequencies);
		list_scroller.setAlignmentX(LEFT_ALIGNMENT);
		c.fill = GridBagConstraints.BOTH;
		c.anchor = GridBagConstraints.WEST;
		c.gridx = 0;
		c.gridy = 1;
		c.gridwidth = 6;
		c.gridheight = 2;
		c.weightx = 1;
		c.weighty = 1;
		pane.add(list_scroller, c);

		add = new JButton("Add");
		add.setActionCommand("add");
		add.addActionListener(this);
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.gridx = 0;
		c.gridy = 3;
		c.gridwidth = 2;
		c.gridheight = 1;
		c.weightx = 0;
		c.weighty = 0;
		pane.add(add, c);

		edit = new JButton("Edit");
		edit.setActionCommand("edit");
		edit.addActionListener(this);
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.gridx = 2;
		c.gridy = 3;
		c.gridwidth = 2;
		c.gridheight = 1;
		c.weightx = 0;
		c.weighty = 0;
		pane.add(edit, c);

		remove = new JButton("Remove");
		remove.setActionCommand("remove");
		remove.addActionListener(this);
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.gridx = 4;
		c.gridy = 3;
		c.gridwidth = 2;
		c.gridheight = 1;
		c.weightx = 0;
		c.weighty = 0;
		pane.add(remove, c);

		ok = new JButton("OK");
		ok.setActionCommand("ok");
		ok.addActionListener(this);
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.gridx = 0;
		c.gridy = 4;
		c.gridwidth = 3;
		c.gridheight = 1;
		c.weightx = 0;
		c.weighty = 0;
		pane.add(ok, c);

		cancel = new JButton("Cancel");
		cancel.setActionCommand("cancel");
		cancel.addActionListener(this);
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.gridx = 3;
		c.gridy = 4;
		c.gridwidth = 3;
		c.gridheight = 1;
		c.weightx = 0;
		c.weighty = 0;
		pane.add(cancel, c);

		pack();
		setLocationRelativeTo(frame);
	}

	public static void show(Component frameComp) {
		Frame	frame = JOptionPane.getFrameForComponent(frameComp);
		AltosConfigFreqUI	dialog;

		dialog = new AltosConfigFreqUI(frame, AltosUIPreferences.common_frequencies());
		dialog.setVisible(true);
	}

}

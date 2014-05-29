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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.io.*;
import org.altusmetrum.altoslib_4.*;

public class AltosCSVUI
	extends AltosUIDialog
	implements ActionListener
{
	JFileChooser		csv_chooser;
	JPanel			accessory;
	JComboBox<String>	combo_box;
	Iterable<AltosState>	states;
	AltosWriter		writer;

	static String[]		combo_box_items = { "Comma Separated Values (.CSV)", "Googleearth Data (.KML)" };

	void set_default_file() {
		File	current = csv_chooser.getSelectedFile();
		String	current_name = current.getName();
		String	new_name = null;
		String	selected = (String) combo_box.getSelectedItem();

		if (selected.contains("CSV"))
			new_name = AltosLib.replace_extension(current_name, ".csv");
		else if (selected.contains("KML"))
			new_name = AltosLib.replace_extension(current_name, ".kml");
		if (new_name != null)
			csv_chooser.setSelectedFile(new File(new_name));
	}

	public void actionPerformed(ActionEvent e) {
		if (e.getActionCommand().equals("comboBoxChanged"))
			set_default_file();
	}

	public AltosCSVUI(JFrame frame, AltosStateIterable states, File source_file) {
		this.states = states;
		csv_chooser = new JFileChooser(source_file);

		accessory = new JPanel();
		accessory.setLayout(new GridBagLayout());

		GridBagConstraints	c = new GridBagConstraints();
		c.fill = GridBagConstraints.NONE;
		c.weightx = 1;
		c.weighty = 0;
		c.insets = new Insets (4, 4, 4, 4);

		JLabel accessory_label = new JLabel("Export File Type");
		c.gridx = 0;
		c.gridy = 0;
		accessory.add(accessory_label, c);

		combo_box = new JComboBox<String>(combo_box_items);
		combo_box.addActionListener(this);
		c.gridx = 0;
		c.gridy = 1;
		accessory.add(combo_box, c);

		csv_chooser.setAccessory(accessory);
		csv_chooser.setSelectedFile(source_file);
		set_default_file();
		int ret = csv_chooser.showSaveDialog(frame);
		if (ret == JFileChooser.APPROVE_OPTION) {
			File file = csv_chooser.getSelectedFile();
			String type = (String) combo_box.getSelectedItem();
			try {
				if (type.contains("CSV"))
					writer = new AltosCSV(file);
				else
					writer = new AltosKML(file);
				writer.write(states);
				writer.close();
			} catch (FileNotFoundException ee) {
				JOptionPane.showMessageDialog(frame,
							      ee.getMessage(),
							      "Cannot open file",
							      JOptionPane.ERROR_MESSAGE);
			}
		}
	}
}

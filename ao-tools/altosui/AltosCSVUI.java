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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.table.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.util.concurrent.LinkedBlockingQueue;

public class AltosCSVUI
	extends JDialog
	implements Runnable, ActionListener
{
	JFrame			frame;
	Thread			thread;
	AltosRecordIterable	iterable;
	AltosWriter		writer;
	JFileChooser		csv_chooser;
	JComboBox		combo_box;

	static String[]		combo_box_items = { "CSV", "KML" };

	void set_default_file() {
		File	current = csv_chooser.getSelectedFile();
		String	current_name = current.getName();
		String	new_name = null;
		String	selected = (String) combo_box.getSelectedItem();

		if (selected.equals("CSV"))
			new_name = Altos.replace_extension(current_name, ".csv");
		else if (selected.equals("KML"))
			new_name = Altos.replace_extension(current_name, ".kml");
		if (new_name != null)
			csv_chooser.setSelectedFile(new File(new_name));
	}

	public void run() {
		AltosLogfileChooser	chooser;

		chooser = new AltosLogfileChooser(frame);
		iterable = chooser.runDialog();
		if (iterable == null)
			return;

		csv_chooser = new JFileChooser(chooser.file());
		combo_box = new JComboBox(combo_box_items);
		combo_box.addActionListener(this);
		csv_chooser.setAccessory(combo_box);
		csv_chooser.setSelectedFile(chooser.file());
		set_default_file();
		int ret = csv_chooser.showSaveDialog(frame);
		if (ret == JFileChooser.APPROVE_OPTION) {
			File 	file = csv_chooser.getSelectedFile();
			String	type = (String) combo_box.getSelectedItem();
			try {
				if (type.equals("CSV"))
					writer = new AltosCSV(file);
				else
					writer = new AltosKML(file);
			} catch (FileNotFoundException ee) {
				JOptionPane.showMessageDialog(frame,
							      file.getName(),
							      "Cannot open file",
							      JOptionPane.ERROR_MESSAGE);
			}
			writer.write(iterable);
			writer.close();
		}
	}

	public void actionPerformed(ActionEvent e) {
		System.out.printf("command %s param %s\n", e.getActionCommand(), e.paramString());
		if (e.getActionCommand().equals("comboBoxChanged"))
			set_default_file();
	}

	public AltosCSVUI(JFrame in_frame) {
		frame = in_frame;
		thread = new Thread(this);
		thread.start();
	}
}

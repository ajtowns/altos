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

import altosui.AltosLogfileChooser;
import altosui.AltosCSV;

public class AltosCSVUI
	extends JDialog
	implements Runnable, ActionListener
{
	JFrame		frame;
	Thread		thread;
	AltosReader	reader;
	AltosCSV	writer;

	public void run() {
		AltosLogfileChooser	chooser;

		chooser = new AltosLogfileChooser(frame);
		reader = chooser.runDialog();
		if (reader == null)
			return;
		JFileChooser	csv_chooser;

		File file = chooser.file();
		String path = file.getPath();
		int dot = path.lastIndexOf(".");
		if (dot >= 0)
			path = path.substring(0,dot);
		path = path.concat(".csv");
		csv_chooser = new JFileChooser(path);
		csv_chooser.setSelectedFile(new File(path));
		int ret = csv_chooser.showSaveDialog(frame);
		if (ret == JFileChooser.APPROVE_OPTION) {
			try {
				writer = new AltosCSV(csv_chooser.getSelectedFile());
			} catch (FileNotFoundException ee) {
				JOptionPane.showMessageDialog(frame,
							      file.getName(),
							      "Cannot open file",
							      JOptionPane.ERROR_MESSAGE);
			}
			writer.write(reader);
			reader.close();
			writer.close();
		}
	}

	public void actionPerformed(ActionEvent e) {
	}

	public AltosCSVUI(JFrame in_frame) {
		frame = in_frame;
		thread = new Thread(this);
		thread.start();
	}
}

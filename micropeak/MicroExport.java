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

import java.io.*;
import java.util.ArrayList;

import java.awt.*;
import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

public class MicroExport extends JFileChooser {

	JFrame		frame;
	MicroData	data;

	public static void export(File file, MicroData data) throws FileNotFoundException, IOException {
		FileWriter fw = new FileWriter(file);
		data.export(fw);
		fw.close();
	}

	public boolean runDialog() {
		int	ret;

		setSelectedFile(new File(AltosLib.replace_extension(data.name, ".csv")));
		for (;;) {
			ret = showSaveDialog(frame);
			if (ret != APPROVE_OPTION)
				return false;
			File	file;
			String	filename;
			file = getSelectedFile();
			if (file == null)
				continue;
			if (!file.getName().contains(".")) {
				String fullname = file.getPath();
				file = new File(fullname.concat(".csv"));
			}
			filename = file.getName();
			if (file.exists()) {
				if (file.isDirectory()) {
					JOptionPane.showMessageDialog(frame,
								      String.format("\"%s\" is a directory",
										    filename),
								      "Directory",
								      JOptionPane.ERROR_MESSAGE);
					continue;
				}
				int r = JOptionPane.showConfirmDialog(frame,
								      String.format("\"%s\" already exists. Overwrite?",
										    filename),
								      "Overwrite file?",
								      JOptionPane.YES_NO_OPTION);
				if (r != JOptionPane.YES_OPTION)
					continue;
							      
				if (!file.canWrite()) {
					JOptionPane.showMessageDialog(frame,
								      String.format("\"%s\" is not writable",
										    filename),
								      "File not writable",
								      JOptionPane.ERROR_MESSAGE);
					continue;
				}
			}
			try {
				export(file, data);
				return true;
			} catch (FileNotFoundException fe) {
				JOptionPane.showMessageDialog(frame,
							      fe.getMessage(),
							      "Cannot create file",
							      JOptionPane.ERROR_MESSAGE);
			} catch (IOException ioe) {
			}
		}
	}

	public MicroExport(JFrame frame, MicroData data) {
		this.frame = frame;
		this.data = data;
		setDialogTitle("Export MicroPeak Data File");
		setFileFilter(new FileNameExtensionFilter("MicroPeak CSV file",
							  "csv"));
		setCurrentDirectory(AltosUIPreferences.logdir());
	}
}

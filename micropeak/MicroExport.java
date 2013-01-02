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
import org.altusmetrum.AltosLib.*;
import org.altusmetrum.altosuilib.*;

public class MicroExport extends JFileChooser {

	JFrame		frame;
	MicroData	data;

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
				FileWriter fw = new FileWriter(file);
				PrintWriter pw = new PrintWriter(fw);
				pw.printf(" Time, Press, Height,  Speed,  Accel\n");
				for (MicroDataPoint point : data.points()) {
					pw.printf("%5.2f,%6.0f,%7.1f,%7.2f,%7.2f\n",
						  point.time, point.pressure, point.height, point.speed, point.accel);
				}
				fw.close();
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

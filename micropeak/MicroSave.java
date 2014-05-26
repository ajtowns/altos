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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import java.io.*;
import java.util.concurrent.*;
import java.util.*;
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

public class MicroSave extends JFileChooser {

	JFrame		frame;
	MicroData	data;

	public static void save(File file, MicroData data) throws FileNotFoundException, IOException {
		FileOutputStream fos = new FileOutputStream(file);
		data.save(fos);
		fos.close();
	}

	public boolean runDialog() {
		int	ret;

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
				file = new File(fullname.concat(".mpd"));
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
				save(file, data);
				AltosUIPreferences.set_last_logdir(file.getParentFile());
				data.set_name(filename);
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

	public MicroSave(JFrame frame, MicroData data) {
		this.frame = frame;
		this.data = data;
		setDialogTitle("Save MicroPeak Data File");
		setFileFilter(new FileNameExtensionFilter("MicroPeak data file",
							  "mpd"));
		setCurrentDirectory(AltosUIPreferences.last_logdir());
		setSelectedFile(MicroFile.make());
	}
}

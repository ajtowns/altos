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

import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import java.io.*;
import org.altusmetrum.altoslib_4.*;

public class AltosDataChooser extends JFileChooser {
	JFrame	frame;
	String	filename;
	File	file;

	public String filename() {
		return filename;
	}

	public File file() {
		return file;
	}

	public AltosStateIterable runDialog() {
		int	ret;

		ret = showOpenDialog(frame);
		if (ret == APPROVE_OPTION) {
			file = getSelectedFile();
			if (file == null)
				return null;
			filename = file.getName();
			try {
				if (filename.endsWith("eeprom")) {
					FileInputStream in = new FileInputStream(file);
					return new AltosEepromFile(in);
				} else if (filename.endsWith("telem")) {
					FileInputStream in = new FileInputStream(file);
					return new AltosTelemetryFile(in);
				} else {
					throw new FileNotFoundException();
				}
			} catch (FileNotFoundException fe) {
				JOptionPane.showMessageDialog(frame,
							      fe.getMessage(),
							      "Cannot open file",
							      JOptionPane.ERROR_MESSAGE);
			}
		}
		return null;
	}

	public AltosDataChooser(JFrame in_frame) {
		frame = in_frame;
		setDialogTitle("Select Flight Record File");
		setFileFilter(new FileNameExtensionFilter("On-board Log file",
							  "eeprom"));
		setFileFilter(new FileNameExtensionFilter("Telemetry file",
							  "telem"));
		setFileFilter(new FileNameExtensionFilter("Flight data file",
							  "telem", "eeprom"));
		setCurrentDirectory(AltosUIPreferences.logdir());
	}
}

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
import org.altusmetrum.AltosLib.*;

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

	public AltosRecordIterable runDialog() {
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
					return new AltosEepromIterable(in);
				} else if (filename.endsWith("telem")) {
					FileInputStream in = new FileInputStream(file);
					return new AltosTelemetryIterable(in);
				} else if (filename.endsWith("mega")) {
					FileInputStream in = new FileInputStream(file);
					return new AltosEepromMegaIterable(in);
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
		setFileFilter(new FileNameExtensionFilter("Flight data file",
							  "telem", "eeprom", "mega"));
		setCurrentDirectory(AltosUIPreferences.logdir());
	}
}

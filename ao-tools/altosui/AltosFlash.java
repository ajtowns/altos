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

import altosui.AltosHexfile;

public class AltosFlash implements Runnable {
	File		file;
	FileInputStream	input;
	Thread		thread;
	AltosHexfile	image;
	JFrame		frame;

	public void run() {
		try {
			image = new AltosHexfile(input);
			System.out.printf("read file start %d length %d\n",
					  image.address, image.data.length);
		} catch (IOException e) {
			JOptionPane.showMessageDialog(frame,
						      file,
						      e.getMessage(),
						      JOptionPane.ERROR_MESSAGE);
		}
	}

	public AltosFlash(JFrame in_frame) {
		frame = in_frame;

		JFileChooser	hexfile_chooser = new JFileChooser();

		hexfile_chooser.setDialogTitle("Select Flash Image");
		hexfile_chooser.setFileFilter(new FileNameExtensionFilter("Flash Image", "ihx"));
		int returnVal = hexfile_chooser.showOpenDialog(frame);

		if (returnVal == JFileChooser.APPROVE_OPTION) {
			file = hexfile_chooser.getSelectedFile();
			if (file != null) {
				try {
					input = new FileInputStream(file);
					thread = new Thread(this);
					thread.start();
				} catch (FileNotFoundException ee) {
					JOptionPane.showMessageDialog(frame,
								      file,
								      "Cannot open flash file",
								      JOptionPane.ERROR_MESSAGE);
				}
			}
		}
	}
}
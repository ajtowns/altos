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
import altosui.AltosFlash;

public class AltosFlashUI implements Runnable {
	File		file;
	Thread		thread;
	JFrame		frame;
	AltosDevice	debug_dongle;
	AltosFlash	flash;

	public void run() {
		flash = new AltosFlash(file, debug_dongle);
		try {
			flash.open();
			if (!flash.check_rom_config()) {
				AltosRomconfigUI romconfig_ui = new AltosRomconfigUI (frame);
				romconfig_ui.showDialog();
				AltosRomconfig romconfig = romconfig_ui.romconfig();
				if (romconfig == null)
					return;
				flash.set_romconfig(romconfig);
			}
			flash.flash();
		} catch (FileNotFoundException ee) {
			JOptionPane.showMessageDialog(frame,
						      "Cannot open image",
						      file.toString(),
						      JOptionPane.ERROR_MESSAGE);
			return;
		} catch (IOException e) {
			JOptionPane.showMessageDialog(frame,
						      e.getMessage(),
						      file.toString(),
						      JOptionPane.ERROR_MESSAGE);
			return;
		} catch (InterruptedException ie) {
		}
	}

	public AltosFlashUI(JFrame in_frame) {
		frame = in_frame;

		debug_dongle = AltosDeviceDialog.show(frame, AltosDevice.Any);

		if (debug_dongle == null)
			return;

		JFileChooser	hexfile_chooser = new JFileChooser();

		hexfile_chooser.setDialogTitle("Select Flash Image");
		hexfile_chooser.setFileFilter(new FileNameExtensionFilter("Flash Image", "ihx"));
		int returnVal = hexfile_chooser.showOpenDialog(frame);

		if (returnVal != JFileChooser.APPROVE_OPTION)
			return;

		file = hexfile_chooser.getSelectedFile();

		thread = new Thread(this);
		thread.start();
	}
}
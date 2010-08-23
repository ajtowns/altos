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

public class AltosFlash {
	File		file;
	FileInputStream	input;
	Thread		thread;
	AltosHexfile	image;
	JFrame		frame;
	AltosDevice	debug_dongle;
	AltosDebug	debug;
	AltosRomconfig	rom_config;

	public void flash() throws IOException, FileNotFoundException, InterruptedException {
		if (!check_rom_config())
			throw new IOException("Invalid rom config settings");
		rom_config.write(image);
	}

	public boolean check_rom_config() {
		if (rom_config == null)
			rom_config = debug.romconfig();
		return rom_config != null && rom_config.valid();
	}

	public void set_romconfig (AltosRomconfig romconfig) {
		rom_config = romconfig;
	}

	public void open() throws IOException, FileNotFoundException, InterruptedException {
		input = new FileInputStream(file);
		image = new AltosHexfile(input);
		debug.open(debug_dongle);
		if (!debug.check_connection())
			throw new IOException("Debug port not connected");
	}

	public AltosFlash(File in_file, AltosDevice in_debug_dongle) {
		file = in_file;
		debug_dongle = in_debug_dongle;
		debug = new AltosDebug();
	}
}
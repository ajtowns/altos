/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

import java.io.*;
import java.util.*;
import java.awt.Component;
import javax.swing.*;
import org.altusmetrum.altoslib_4.*;

public class AltosUIPreferences extends AltosPreferences {

	/* font size preferences name */
	final static String fontSizePreference = "FONT-SIZE";

	/* Look&Feel preference name */
	final static String lookAndFeelPreference = "LOOK-AND-FEEL";

	/* Window position preference name */
	final static String positionPreference = "POSITION";

	/* UI Component to pop dialogs up */
	static Component component;

	static LinkedList<AltosFontListener> font_listeners;

	static int font_size = AltosUILib.font_size_medium;

	static LinkedList<AltosUIListener> ui_listeners;

	static String look_and_feel = null;

	/* Serial debug */
	public static boolean serial_debug;

	static LinkedList<AltosPositionListener> position_listeners;

	public static int position = AltosUILib.position_top_left;

	public static void init() {
		AltosPreferences.init(new AltosUIPreferencesBackend());

		font_listeners = new LinkedList<AltosFontListener>();

		font_size = backend.getInt(fontSizePreference, AltosUILib.font_size_medium);
		AltosUILib.set_fonts(font_size);
		look_and_feel = backend.getString(lookAndFeelPreference, UIManager.getSystemLookAndFeelClassName());

		ui_listeners = new LinkedList<AltosUIListener>();
		serial_debug = backend.getBoolean(serialDebugPreference, false);

		AltosLink.set_debug(serial_debug);

		position = backend.getInt(positionPreference, AltosUILib.position_top_left);
		position_listeners = new LinkedList<AltosPositionListener>();
	}

	static { init(); }

	public static void set_component(Component in_component) {
		component = in_component;
	}

	private static boolean check_dir(File dir) {
		if (!dir.exists()) {
			if (!dir.mkdirs()) {
				JOptionPane.showMessageDialog(component,
							      dir.getName(),
							      "Cannot create directory",
							      JOptionPane.ERROR_MESSAGE);
				return false;
			}
		} else if (!dir.isDirectory()) {
			JOptionPane.showMessageDialog(component,
						      dir.getName(),
						      "Is not a directory",
						      JOptionPane.ERROR_MESSAGE);
			return false;
		}
		return true;
	}

	/* Configure the log directory. This is where all telemetry and eeprom files
	 * will be written to, and where replay will look for telemetry files
	 */
	public static void ConfigureLog() {
		JFileChooser	logdir_chooser = new JFileChooser(logdir.getParentFile());

		logdir_chooser.setDialogTitle("Configure Data Logging Directory");
		logdir_chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);

		if (logdir_chooser.showDialog(component, "Select Directory") == JFileChooser.APPROVE_OPTION) {
			File dir = logdir_chooser.getSelectedFile();
			if (check_dir(dir))
				set_logdir(dir);
		}
	}
	public static int font_size() {
		synchronized (backend) {
			return font_size;
		}
	}

	static void set_fonts() {
	}

	public static void set_font_size(int new_font_size) {
		synchronized (backend) {
			font_size = new_font_size;
			backend.putInt(fontSizePreference, font_size);
			flush_preferences();
			AltosUILib.set_fonts(font_size);
			for (AltosFontListener l : font_listeners)
				l.font_size_changed(font_size);
		}
	}

	public static void register_font_listener(AltosFontListener l) {
		synchronized (backend) {
			font_listeners.add(l);
		}
	}

	public static void unregister_font_listener(AltosFontListener l) {
		synchronized (backend) {
			font_listeners.remove(l);
		}
	}

	public static void set_look_and_feel(String new_look_and_feel) {
		try {
			UIManager.setLookAndFeel(new_look_and_feel);
		} catch (Exception e) {
		}
		synchronized(backend) {
			look_and_feel = new_look_and_feel;
			backend.putString(lookAndFeelPreference, look_and_feel);
			flush_preferences();
			for (AltosUIListener l : ui_listeners)
				l.ui_changed(look_and_feel);
		}
	}

	public static String look_and_feel() {
		synchronized (backend) {
			return look_and_feel;
		}
	}

	public static void register_ui_listener(AltosUIListener l) {
		synchronized(backend) {
			ui_listeners.add(l);
		}
	}

	public static void unregister_ui_listener(AltosUIListener l) {
		synchronized (backend) {
			ui_listeners.remove(l);
		}
	}
	public static void set_serial_debug(boolean new_serial_debug) {
		AltosLink.set_debug(new_serial_debug);
		synchronized (backend) {
			serial_debug = new_serial_debug;
			backend.putBoolean(serialDebugPreference, serial_debug);
			flush_preferences();
		}
	}

	public static boolean serial_debug() {
		synchronized (backend) {
			return serial_debug;
		}
	}

	public static void register_position_listener(AltosPositionListener l) {
		synchronized(backend) {
			position_listeners.add(l);
		}
	}

	public static void unregister_position_listener(AltosPositionListener l) {
		synchronized (backend) {
			position_listeners.remove(l);
		}
	}

	public static void set_position(int new_position) {
		synchronized (backend) {
			position = new_position;
			backend.putInt(positionPreference, position);
			flush_preferences();
			for (AltosPositionListener l : position_listeners)
				l.position_changed(position);
		}
	}

	public static int position() {
		synchronized (backend) {
			return position;
		}
	}
}

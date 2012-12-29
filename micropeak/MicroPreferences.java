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

package org.altusmetrum.micropeak;

import java.io.*;
import java.util.*;
import java.awt.Component;
import javax.swing.*;
import java.awt.*;
import org.altusmetrum.AltosLib.*;

public class MicroPreferences extends AltosPreferences {

	static final int tab_elt_pad = 5;

	static Font label_font;
	static Font value_font;
	static Font status_font;
	static Font table_label_font;
	static Font table_value_font;

	final static int font_size_small = 1;
	final static int font_size_medium = 2;
	final static int font_size_large = 3;

	static void set_fonts(int size) {
		int	brief_size;
		int	table_size;
		int	status_size;

		switch (size) {
		case font_size_small:
			brief_size = 16;
			status_size = 18;
			table_size = 11;
			break;
		default:
		case font_size_medium:
			brief_size = 22;
			status_size = 24;
			table_size = 14;
			break;
		case font_size_large:
			brief_size = 26;
			status_size = 30;
			table_size = 17;
			break;
		}
		label_font = new Font("Dialog", Font.PLAIN, brief_size);
		value_font = new Font("Monospaced", Font.PLAIN, brief_size);
		status_font = new Font("SansSerif", Font.BOLD, status_size);
		table_label_font = new Font("SansSerif", Font.PLAIN, table_size);
		table_value_font = new Font("Monospaced", Font.PLAIN, table_size);
	}

	/* font size preferences name */
	final static String fontSizePreference = "FONT-SIZE";

	/* Look&Feel preference name */
	final static String lookAndFeelPreference = "LOOK-AND-FEEL";

	/* UI Component to pop dialogs up */
	static Component component;

	static LinkedList<MicroFontListener> font_listeners;

	static int font_size = font_size_medium;

	static LinkedList<MicroUIListener> ui_listeners;

	static String look_and_feel = null;

	/* Serial debug */
	static boolean serial_debug;

	public static void init() {
		AltosPreferences.init(new MicroPreferencesBackend());

		font_listeners = new LinkedList<MicroFontListener>();

		font_size = backend.getInt(fontSizePreference, font_size_medium);
		set_fonts(font_size);
		look_and_feel = backend.getString(lookAndFeelPreference, UIManager.getSystemLookAndFeelClassName());

		ui_listeners = new LinkedList<MicroUIListener>();
		serial_debug = backend.getBoolean(serialDebugPreference, false);
	}

	static { init(); }

	static void set_component(Component in_component) {
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
			set_fonts(font_size);
			for (MicroFontListener l : font_listeners)
				l.font_size_changed(font_size);
		}
	}

	public static void register_font_listener(MicroFontListener l) {
		synchronized (backend) {
			font_listeners.add(l);
		}
	}

	public static void unregister_font_listener(MicroFontListener l) {
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
			for (MicroUIListener l : ui_listeners)
				l.ui_changed(look_and_feel);
		}
	}

	public static String look_and_feel() {
		synchronized (backend) {
			return look_and_feel;
		}
	}

	public static void register_ui_listener(MicroUIListener l) {
		synchronized(backend) {
			ui_listeners.add(l);
		}
	}

	public static void unregister_ui_listener(MicroUIListener l) {
		synchronized (backend) {
			ui_listeners.remove(l);
		}
	}
	public static void set_serial_debug(boolean new_serial_debug) {
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

}

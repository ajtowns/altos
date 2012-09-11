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

package altosui;

import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.util.concurrent.LinkedBlockingQueue;
import java.awt.Component;
import javax.swing.*;
import javax.swing.filechooser.FileSystemView;
import org.altusmetrum.AltosLib.*;

public class AltosUIPreferences extends AltosPreferences {

	/* font size preferences name */
	final static String fontSizePreference = "FONT-SIZE";

	/* Look&Feel preference name */
	final static String lookAndFeelPreference = "LOOK-AND-FEEL";

	/* UI Component to pop dialogs up */
	static Component component;

	static LinkedList<AltosFontListener> font_listeners;

	static int font_size = Altos.font_size_medium;

	static LinkedList<AltosUIListener> ui_listeners;

	static String look_and_feel = null;

	/* Serial debug */
	static boolean serial_debug;

	public static void init() {
		font_listeners = new LinkedList<AltosFontListener>();

		font_size = preferences.getInt(fontSizePreference, Altos.font_size_medium);
		Altos.set_fonts(font_size);
		look_and_feel = preferences.get(lookAndFeelPreference, UIManager.getSystemLookAndFeelClassName());

		ui_listeners = new LinkedList<AltosUIListener>();
		serial_debug = preferences.getBoolean(serialDebugPreference, false);
		AltosLink.set_debug(serial_debug);
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
		return font_size;
	}

	static void set_fonts() {
	}

	public static void set_font_size(int new_font_size) {
		font_size = new_font_size;
		synchronized (preferences) {
			preferences.putInt(fontSizePreference, font_size);
			flush_preferences();
			Altos.set_fonts(font_size);
			for (AltosFontListener l : font_listeners)
				l.font_size_changed(font_size);
		}
	}

	public static void register_font_listener(AltosFontListener l) {
		synchronized (preferences) {
			font_listeners.add(l);
		}
	}

	public static void unregister_font_listener(AltosFontListener l) {
		synchronized (preferences) {
			font_listeners.remove(l);
		}
	}

	public static void set_look_and_feel(String new_look_and_feel) {
		look_and_feel = new_look_and_feel;
		try {
			UIManager.setLookAndFeel(look_and_feel);
		} catch (Exception e) {
		}
		synchronized(preferences) {
			preferences.put(lookAndFeelPreference, look_and_feel);
			flush_preferences();
			for (AltosUIListener l : ui_listeners)
				l.ui_changed(look_and_feel);
		}
	}

	public static String look_and_feel() {
		return look_and_feel;
	}

	public static void register_ui_listener(AltosUIListener l) {
		synchronized(preferences) {
			ui_listeners.add(l);
		}
	}

	public static void unregister_ui_listener(AltosUIListener l) {
		synchronized (preferences) {
			ui_listeners.remove(l);
		}
	}
	public static void set_serial_debug(boolean new_serial_debug) {
		serial_debug = new_serial_debug;
		AltosLink.set_debug(serial_debug);
		synchronized (preferences) {
			preferences.putBoolean(serialDebugPreference, serial_debug);
			flush_preferences();
		}
	}

	public static boolean serial_debug() {
		return serial_debug;
	}

}
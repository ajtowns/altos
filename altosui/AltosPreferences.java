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
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.util.concurrent.LinkedBlockingQueue;
import java.awt.Component;
import javax.swing.*;
import javax.swing.filechooser.FileSystemView;

class AltosPreferences {
	static Preferences preferences;

	/* logdir preference name */
	final static String logdirPreference = "LOGDIR";

	/* channel preference name */
	final static String channelPreferenceFormat = "CHANNEL-%d";

	/* telemetry format preference name */
	final static String telemetryPreferenceFormat = "TELEMETRY-%d";

	/* voice preference name */
	final static String voicePreference = "VOICE";

	/* callsign preference name */
	final static String callsignPreference = "CALLSIGN";

	/* firmware directory preference name */
	final static String firmwaredirPreference = "FIRMWARE";

	/* serial debug preference name */
	final static String serialDebugPreference = "SERIAL-DEBUG";

	/* Default logdir is ~/TeleMetrum */
	final static String logdirName = "TeleMetrum";

	/* UI Component to pop dialogs up */
	static Component component;

	/* Log directory */
	static File logdir;

	/* Map directory -- hangs of logdir */
	static File mapdir;

	/* Channel (map serial to channel) */
	static Hashtable<Integer, Integer> channels;

	/* Telemetry (map serial to telemetry format) */
	static Hashtable<Integer, Integer> telemetries;

	/* Voice preference */
	static boolean voice;

	/* Callsign preference */
	static String callsign;

	/* Firmware directory */
	static File firmwaredir;

	/* Serial debug */
	static boolean serial_debug;

	public static void init() {
		preferences = Preferences.userRoot().node("/org/altusmetrum/altosui");

		/* Initialize logdir from preferences */
		String logdir_string = preferences.get(logdirPreference, null);
		if (logdir_string != null)
			logdir = new File(logdir_string);
		else {
			/* Use the file system view default directory */
			logdir = new File(FileSystemView.getFileSystemView().getDefaultDirectory(), logdirName);
			if (!logdir.exists())
				logdir.mkdirs();
		}
		mapdir = new File(logdir, "maps");
		if (!mapdir.exists())
			mapdir.mkdirs();

		channels = new Hashtable<Integer,Integer>();

		telemetries = new Hashtable<Integer,Integer>();

		voice = preferences.getBoolean(voicePreference, true);

		callsign = preferences.get(callsignPreference,"N0CALL");

		String firmwaredir_string = preferences.get(firmwaredirPreference, null);
		if (firmwaredir_string != null)
			firmwaredir = new File(firmwaredir_string);
		else
			firmwaredir = null;

		serial_debug = preferences.getBoolean(serialDebugPreference, false);
		AltosSerial.set_debug(serial_debug);
	}

	static { init(); }

	static void set_component(Component in_component) {
		component = in_component;
	}

	static void flush_preferences() {
		try {
			preferences.flush();
		} catch (BackingStoreException ee) {
			if (component != null)
				JOptionPane.showMessageDialog(component,
							      preferences.absolutePath(),
							      "Cannot save prefernces",
							      JOptionPane.ERROR_MESSAGE);
			else
				System.err.printf("Cannot save preferences\n");
		}
	}

	public static void set_logdir(File new_logdir) {
		logdir = new_logdir;
		mapdir = new File(logdir, "maps");
		if (!mapdir.exists())
			mapdir.mkdirs();
		synchronized (preferences) {
			preferences.put(logdirPreference, logdir.getPath());
			flush_preferences();
		}
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

	public static File logdir() {
		return logdir;
	}

	public static File mapdir() {
		return mapdir;
	}

	public static void set_channel(int serial, int new_channel) {
		channels.put(serial, new_channel);
		synchronized (preferences) {
			preferences.putInt(String.format(channelPreferenceFormat, serial), new_channel);
			flush_preferences();
		}
	}

	public static int channel(int serial) {
		if (channels.containsKey(serial))
			return channels.get(serial);
		int channel = preferences.getInt(String.format(channelPreferenceFormat, serial), 0);
		channels.put(serial, channel);
		return channel;
	}

	public static void set_telemetry(int serial, int new_telemetry) {
		telemetries.put(serial, new_telemetry);
		synchronized (preferences) {
			preferences.putInt(String.format(telemetryPreferenceFormat, serial), new_telemetry);
			flush_preferences();
		}
	}

	public static int telemetry(int serial) {
		if (telemetries.containsKey(serial))
			return telemetries.get(serial);
		int telemetry = preferences.getInt(String.format(telemetryPreferenceFormat, serial),
						   Altos.ao_telemetry_full);
		telemetries.put(serial, telemetry);
		return telemetry;
	}

	public static void set_voice(boolean new_voice) {
		voice = new_voice;
		synchronized (preferences) {
			preferences.putBoolean(voicePreference, voice);
			flush_preferences();
		}
	}

	public static boolean voice() {
		return voice;
	}

	public static void set_callsign(String new_callsign) {
		callsign = new_callsign;
		synchronized(preferences) {
			preferences.put(callsignPreference, callsign);
			flush_preferences();
		}
	}

	public static String callsign() {
		return callsign;
	}

	public static void set_firmwaredir(File new_firmwaredir) {
		firmwaredir = new_firmwaredir;
		synchronized (preferences) {
			preferences.put(firmwaredirPreference, firmwaredir.getPath());
			flush_preferences();
		}
	}

	public static File firmwaredir() {
		return firmwaredir;
	}

	public static void set_serial_debug(boolean new_serial_debug) {
		serial_debug = new_serial_debug;
		AltosSerial.set_debug(serial_debug);
		synchronized (preferences) {
			preferences.putBoolean(serialDebugPreference, serial_debug);
			flush_preferences();
		}
	}

	public static boolean serial_debug() {
		return serial_debug;
	}

	public static Preferences bt_devices() {
		return preferences.node("bt_devices");
	}
}

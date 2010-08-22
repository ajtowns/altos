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
	final static String channelPreference = "CHANNEL";

	/* voice preference name */
	final static String voicePreference = "VOICE";

	/* callsign preference name */
	final static String callsignPreference = "CALLSIGN";

	/* Default logdir is ~/TeleMetrum */
	final static String logdirName = "TeleMetrum";

	/* UI Component to pop dialogs up */
	static Component component;

	/* Log directory */
	static File logdir;

	/* Telemetry channel */
	static int channel;

	/* Voice preference */
	static boolean voice;

	static String callsign;

	public static void init(Component ui) {
		preferences = Preferences.userRoot().node("/org/altusmetrum/altosui");

		component = ui;

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

		channel = preferences.getInt(channelPreference, 0);

		voice = preferences.getBoolean(voicePreference, true);

		callsign = preferences.get(callsignPreference,"N0CALL");
	}

	static void flush_preferences() {
		try {
			preferences.flush();
		} catch (BackingStoreException ee) {
			JOptionPane.showMessageDialog(component,
						      preferences.absolutePath(),
						      "Cannot save prefernces",
						      JOptionPane.ERROR_MESSAGE);
		}
	}

	public static void set_logdir(File new_logdir) {
		logdir = new_logdir;
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

	public static void set_channel(int new_channel) {
		channel = new_channel;
		synchronized (preferences) {
			preferences.putInt(channelPreference, channel);
			flush_preferences();
		}
	}

	public static int channel() {
		return channel;
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
}

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

package org.altusmetrum.AltosLib;

import java.io.*;
import java.util.*;
import java.util.prefs.*;
import javax.swing.filechooser.FileSystemView;

public class AltosPreferences {
	public static Preferences preferences;

	/* logdir preference name */
	public final static String logdirPreference = "LOGDIR";

	/* channel preference name */
	public final static String channelPreferenceFormat = "CHANNEL-%d";

	/* frequency preference name */
	public final static String frequencyPreferenceFormat = "FREQUENCY-%d";

	/* telemetry format preference name */
	public final static String telemetryPreferenceFormat = "TELEMETRY-%d";

	/* voice preference name */
	public final static String voicePreference = "VOICE";

	/* callsign preference name */
	public final static String callsignPreference = "CALLSIGN";

	/* firmware directory preference name */
	public final static String firmwaredirPreference = "FIRMWARE";

	/* serial debug preference name */
	public final static String serialDebugPreference = "SERIAL-DEBUG";

	/* scanning telemetry preferences name */
	public final static String scanningTelemetryPreference = "SCANNING-TELEMETRY";

	/* Launcher serial preference name */
	public final static String launcherSerialPreference = "LAUNCHER-SERIAL";

	/* Launcher channel preference name */
	public final static String launcherChannelPreference = "LAUNCHER-CHANNEL";
	
	/* Default logdir is ~/TeleMetrum */
	public final static String logdirName = "TeleMetrum";

	/* Log directory */
	public static File logdir;

	/* Map directory -- hangs of logdir */
	public static File mapdir;

	/* Frequency (map serial to frequency) */
	public static Hashtable<Integer, Double> frequencies;

	/* Telemetry (map serial to telemetry format) */
	public static Hashtable<Integer, Integer> telemetries;

	/* Voice preference */
	public static boolean voice;

	/* Callsign preference */
	public static String callsign;

	/* Firmware directory */
	public static File firmwaredir;

	/* Scanning telemetry */
	public static int scanning_telemetry;

	/* List of frequencies */
	public final static String common_frequencies_node_name = "COMMON-FREQUENCIES";
	public static AltosFrequency[] common_frequencies;

	public final static String	frequency_count = "COUNT";
	public final static String	frequency_format = "FREQUENCY-%d";
	public final static String	description_format = "DESCRIPTION-%d";

	/* Units preference */

	public final static String	unitsPreference = "IMPERIAL-UNITS";

	public static AltosFrequency[] load_common_frequencies() {
		AltosFrequency[] frequencies = null;
		boolean	existing = false;
		try {
			existing = preferences.nodeExists(common_frequencies_node_name);
		} catch (BackingStoreException be) {
			existing = false;
		}
		if (existing) {
			Preferences	node = preferences.node(common_frequencies_node_name);
			int		count = node.getInt(frequency_count, 0);

			frequencies = new AltosFrequency[count];
			for (int i = 0; i < count; i++) {
				double	frequency;
				String	description;

				frequency = node.getDouble(String.format(frequency_format, i), 0.0);
				description = node.get(String.format(description_format, i), null);
				frequencies[i] = new AltosFrequency(frequency, description);
			}
		} else {
			frequencies = new AltosFrequency[10];
			for (int i = 0; i < 10; i++) {
				frequencies[i] = new AltosFrequency(434.550 + i * .1,
									   String.format("Channel %d", i));
			}
		}
		return frequencies;
	}

	public static void save_common_frequencies(AltosFrequency[] frequencies) {
		Preferences	node = preferences.node(common_frequencies_node_name);

		node.putInt(frequency_count, frequencies.length);
		for (int i = 0; i < frequencies.length; i++) {
			node.putDouble(String.format(frequency_format, i), frequencies[i].frequency);
			node.put(String.format(description_format, i), frequencies[i].description);
		}
	}
	public static int launcher_serial;

	public static int launcher_channel;

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

		frequencies = new Hashtable<Integer, Double>();

		telemetries = new Hashtable<Integer,Integer>();

		voice = preferences.getBoolean(voicePreference, true);

		callsign = preferences.get(callsignPreference,"N0CALL");

		scanning_telemetry = preferences.getInt(scanningTelemetryPreference,(1 << AltosLib.ao_telemetry_standard));

		launcher_serial = preferences.getInt(launcherSerialPreference, 0);

		launcher_channel = preferences.getInt(launcherChannelPreference, 0);

		String firmwaredir_string = preferences.get(firmwaredirPreference, null);
		if (firmwaredir_string != null)
			firmwaredir = new File(firmwaredir_string);
		else
			firmwaredir = null;

		common_frequencies = load_common_frequencies();

		AltosConvert.imperial_units = preferences.getBoolean(unitsPreference, false);
	}

	static { init(); }

	public static void flush_preferences() {
		try {
			preferences.flush();
		} catch (BackingStoreException ee) {
/*
			if (component != null)
				JOptionPane.showMessageDialog(component,
							      preferences.absolutePath(),
							      "Cannot save prefernces",
							      JOptionPane.ERROR_MESSAGE);
			else
*/
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

	public static File logdir() {
		return logdir;
	}

	public static File mapdir() {
		return mapdir;
	}

	public static void set_frequency(int serial, double new_frequency) {
		frequencies.put(serial, new_frequency);
		synchronized (preferences) {
			preferences.putDouble(String.format(frequencyPreferenceFormat, serial), new_frequency);
			flush_preferences();
		}
	}

	public static double frequency(int serial) {
		if (frequencies.containsKey(serial))
			return frequencies.get(serial);
		double frequency = preferences.getDouble(String.format(frequencyPreferenceFormat, serial), 0);
		if (frequency == 0.0) {
			int channel = preferences.getInt(String.format(channelPreferenceFormat, serial), 0);
			frequency = AltosConvert.radio_channel_to_frequency(channel);
		}
		frequencies.put(serial, frequency);
		return frequency;
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
						   AltosLib.ao_telemetry_standard);
		telemetries.put(serial, telemetry);
		return telemetry;
	}

	public static void set_scanning_telemetry(int new_scanning_telemetry) {
		scanning_telemetry = new_scanning_telemetry;
		synchronized (preferences) {
			preferences.putInt(scanningTelemetryPreference, scanning_telemetry);
			flush_preferences();
		}
	}

	public static int scanning_telemetry() {
		return scanning_telemetry;
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

	public static void set_launcher_serial(int new_launcher_serial) {
		launcher_serial = new_launcher_serial;
		synchronized (preferences) {
			preferences.putInt(launcherSerialPreference, launcher_serial);
			flush_preferences();
		}
	}

	public static int launcher_serial() {
		return launcher_serial;
	}

	public static void set_launcher_channel(int new_launcher_channel) {
		launcher_channel = new_launcher_channel;
		synchronized (preferences) {
			preferences.putInt(launcherChannelPreference, launcher_channel);
			flush_preferences();
		}
	}

	public static int launcher_channel() {
		return launcher_channel;
	}
	
	public static Preferences bt_devices() {
		return preferences.node("bt_devices");
	}

	public static AltosFrequency[] common_frequencies() {
		return common_frequencies;
	}

	public static void set_common_frequencies(AltosFrequency[] frequencies) {
		common_frequencies = frequencies;
		synchronized(preferences) {
			save_common_frequencies(frequencies);
			flush_preferences();
		}
	}

	public static void add_common_frequency(AltosFrequency frequency) {
		AltosFrequency[]	new_frequencies = new AltosFrequency[common_frequencies.length + 1];
		int			i;

		for (i = 0; i < common_frequencies.length; i++) {
			if (frequency.frequency == common_frequencies[i].frequency)
				return;
			if (frequency.frequency < common_frequencies[i].frequency)
				break;
			new_frequencies[i] = common_frequencies[i];
		}
		new_frequencies[i] = frequency;
		for (; i < common_frequencies.length; i++)
			new_frequencies[i+1] = common_frequencies[i];
		set_common_frequencies(new_frequencies);
	}

	public static boolean imperial_units() {
		return AltosConvert.imperial_units;
	}

	public static void set_imperial_units(boolean imperial_units) {
		AltosConvert.imperial_units = imperial_units;
		synchronized (preferences) {
			preferences.putBoolean(unitsPreference, imperial_units);
			flush_preferences();
		}
	}
}

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

package org.altusmetrum.altoslib_4;

import java.io.*;
import java.util.*;

public class AltosPreferences {
	public static AltosPreferencesBackend backend = null;

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

	/* Last log directory - use this next time we open or save something */
	public static File last_logdir;

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
		existing = backend.nodeExists(common_frequencies_node_name);

		if (existing) {
			AltosPreferencesBackend	node = backend.node(common_frequencies_node_name);
			int		count = node.getInt(frequency_count, 0);

			frequencies = new AltosFrequency[count];
			for (int i = 0; i < count; i++) {
				double	frequency;
				String	description;

				frequency = node.getDouble(String.format(frequency_format, i), 0.0);
				description = node.getString(String.format(description_format, i), null);
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
		AltosPreferencesBackend	node = backend.node(common_frequencies_node_name);

		node.putInt(frequency_count, frequencies.length);
		for (int i = 0; i < frequencies.length; i++) {
			node.putDouble(String.format(frequency_format, i), frequencies[i].frequency);
			node.putString(String.format(description_format, i), frequencies[i].description);
		}
	}
	public static int launcher_serial;

	public static int launcher_channel;

	public static void init(AltosPreferencesBackend in_backend) {
		backend = in_backend;

		/* Initialize logdir from preferences */
		String logdir_string = backend.getString(logdirPreference, null);
		if (logdir_string != null)
			logdir = new File(logdir_string);
		else {
			logdir = new File(backend.homeDirectory(), logdirName);
			if (!logdir.exists())
				logdir.mkdirs();
		}
		mapdir = new File(logdir, "maps");
		if (!mapdir.exists())
			mapdir.mkdirs();

		frequencies = new Hashtable<Integer, Double>();

		telemetries = new Hashtable<Integer,Integer>();

		voice = backend.getBoolean(voicePreference, true);

		callsign = backend.getString(callsignPreference,"N0CALL");

		scanning_telemetry = backend.getInt(scanningTelemetryPreference,(1 << AltosLib.ao_telemetry_standard));

		launcher_serial = backend.getInt(launcherSerialPreference, 0);

		launcher_channel = backend.getInt(launcherChannelPreference, 0);

		String firmwaredir_string = backend.getString(firmwaredirPreference, null);
		if (firmwaredir_string != null)
			firmwaredir = new File(firmwaredir_string);
		else
			firmwaredir = null;

		common_frequencies = load_common_frequencies();

		AltosConvert.imperial_units = backend.getBoolean(unitsPreference, false);
	}

	public static void flush_preferences() {
		backend.flush();
	}

	public static void set_logdir(File new_logdir) {
		synchronized (backend) {
			logdir = new_logdir;
			mapdir = new File(logdir, "maps");
			if (!mapdir.exists())
				mapdir.mkdirs();
			backend.putString(logdirPreference, logdir.getPath());
			flush_preferences();
		}
	}

	public static File logdir() {
		synchronized (backend) {
			return logdir;
		}
	}

	public static File last_logdir() {
		synchronized (backend) {
			if (last_logdir == null)
				last_logdir = logdir;
			return last_logdir;
		}
	}

	public static void set_last_logdir(File file) {
		synchronized(backend) {
			if (file != null && !file.isDirectory())
				file = file.getParentFile();
			if (file == null)
				file = new File(".");
			last_logdir = file;
		}
	}

	public static File mapdir() {
		synchronized (backend) {
			return mapdir;
		}
	}

	public static void set_frequency(int serial, double new_frequency) {
		synchronized (backend) {
			frequencies.put(serial, new_frequency);
			backend.putDouble(String.format(frequencyPreferenceFormat, serial), new_frequency);
			flush_preferences();
		}
	}

	public static double frequency(int serial) {
		synchronized (backend) {
			if (frequencies.containsKey(serial))
				return frequencies.get(serial);
			double frequency = backend.getDouble(String.format(frequencyPreferenceFormat, serial), 0);
			if (frequency == 0.0) {
				int channel = backend.getInt(String.format(channelPreferenceFormat, serial), 0);
				frequency = AltosConvert.radio_channel_to_frequency(channel);
			}
			frequencies.put(serial, frequency);
			return frequency;
		}
	}

	public static void set_telemetry(int serial, int new_telemetry) {
		synchronized (backend) {
			telemetries.put(serial, new_telemetry);
			backend.putInt(String.format(telemetryPreferenceFormat, serial), new_telemetry);
			flush_preferences();
		}
	}

	public static int telemetry(int serial) {
		synchronized (backend) {
			if (telemetries.containsKey(serial))
				return telemetries.get(serial);
			int telemetry = backend.getInt(String.format(telemetryPreferenceFormat, serial),
						   AltosLib.ao_telemetry_standard);
			telemetries.put(serial, telemetry);
			return telemetry;
		}
	}

	public static void set_scanning_telemetry(int new_scanning_telemetry) {
		synchronized (backend) {
			scanning_telemetry = new_scanning_telemetry;
			backend.putInt(scanningTelemetryPreference, scanning_telemetry);
			flush_preferences();
		}
	}

	public static int scanning_telemetry() {
		synchronized (backend) {
			return scanning_telemetry;
		}
	}

	public static void set_voice(boolean new_voice) {
		synchronized (backend) {
			voice = new_voice;
			backend.putBoolean(voicePreference, voice);
			flush_preferences();
		}
	}

	public static boolean voice() {
		synchronized (backend) {
			return voice;
		}
	}

	public static void set_callsign(String new_callsign) {
		synchronized(backend) {
			callsign = new_callsign;
			backend.putString(callsignPreference, callsign);
			flush_preferences();
		}
	}

	public static String callsign() {
		synchronized(backend) {
			return callsign;
		}
	}

	public static void set_firmwaredir(File new_firmwaredir) {
		synchronized (backend) {
			firmwaredir = new_firmwaredir;
			backend.putString(firmwaredirPreference, firmwaredir.getPath());
			flush_preferences();
		}
	}

	public static File firmwaredir() {
		synchronized (backend) {
			return firmwaredir;
		}
	}

	public static void set_launcher_serial(int new_launcher_serial) {
		synchronized (backend) {
			launcher_serial = new_launcher_serial;
			backend.putInt(launcherSerialPreference, launcher_serial);
			flush_preferences();
		}
	}

	public static int launcher_serial() {
		synchronized (backend) {
			return launcher_serial;
		}
	}

	public static void set_launcher_channel(int new_launcher_channel) {
		synchronized (backend) {
			launcher_channel = new_launcher_channel;
			backend.putInt(launcherChannelPreference, launcher_channel);
			flush_preferences();
		}
	}

	public static int launcher_channel() {
		synchronized (backend) {
			return launcher_channel;
		}
	}

	public static AltosPreferencesBackend bt_devices() {
		synchronized (backend) {
			return backend.node("bt_devices");
		}
	}

	public static AltosFrequency[] common_frequencies() {
		synchronized (backend) {
			return common_frequencies;
		}
	}

	public static void set_common_frequencies(AltosFrequency[] frequencies) {
		synchronized(backend) {
			common_frequencies = frequencies;
			save_common_frequencies(frequencies);
			flush_preferences();
		}
	}

	public static void add_common_frequency(AltosFrequency frequency) {
		AltosFrequency[]	old_frequencies = common_frequencies();
		AltosFrequency[]	new_frequencies = new AltosFrequency[old_frequencies.length + 1];
		int			i;

		for (i = 0; i < old_frequencies.length; i++) {
			if (frequency.frequency == old_frequencies[i].frequency)
				return;
			if (frequency.frequency < old_frequencies[i].frequency)
				break;
			new_frequencies[i] = old_frequencies[i];
		}
		new_frequencies[i] = frequency;
		for (; i < old_frequencies.length; i++)
			new_frequencies[i+1] = old_frequencies[i];
		set_common_frequencies(new_frequencies);
	}

	static LinkedList<AltosUnitsListener> units_listeners;

	public static boolean imperial_units() {
		synchronized(backend) {
			return AltosConvert.imperial_units;
		}
	}

	public static void set_imperial_units(boolean imperial_units) {
		synchronized (backend) {
			AltosConvert.imperial_units = imperial_units;
			backend.putBoolean(unitsPreference, imperial_units);
			flush_preferences();
		}
		if (units_listeners != null) {
			for (AltosUnitsListener l : units_listeners) {
				l.units_changed(imperial_units);
			}
		}
	}

	public static void register_units_listener(AltosUnitsListener l) {
		synchronized(backend) {
			if (units_listeners == null)
				units_listeners = new LinkedList<AltosUnitsListener>();
			units_listeners.add(l);
		}
	}

	public static void unregister_units_listener(AltosUnitsListener l) {
		synchronized(backend) {
			units_listeners.remove(l);
		}
	}
}

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

package org.altusmetrum.AltosLib;

import java.util.*;
import java.text.*;
import java.util.concurrent.*;

public class AltosConfigData implements Iterable<String> {

	/* Version information */
	public String	manufacturer;
	public String	product;
	public String	version;
	public int	log_format;
	public int	serial;

	/* Strings returned */
	public LinkedList<String>	lines;

	/* Config information */
	public int	config_major;
	public int	config_minor;
	public int	main_deploy;
	public int	apogee_delay;
	public int	radio_channel;
	public int	radio_setting;
	public int	radio_frequency;
	public String	callsign;
	public int	accel_cal_plus, accel_cal_minus;
	public int	radio_calibration;
	public int	flight_log_max;
	public int	ignite_mode;
	public int	stored_flight;
	public int	storage_size;
	public int	storage_erase_unit;

	public static String get_string(String line, String label) throws  ParseException {
		if (line.startsWith(label)) {
			String	quoted = line.substring(label.length()).trim();

			if (quoted.startsWith("\""))
				quoted = quoted.substring(1);
			if (quoted.endsWith("\""))
				quoted = quoted.substring(0,quoted.length()-1);
			return quoted;
		}
		throw new ParseException("mismatch", 0);
	}

	public static int get_int(String line, String label) throws NumberFormatException, ParseException {
		if (line.startsWith(label)) {
			String tail = line.substring(label.length()).trim();
			String[] tokens = tail.split("\\s+");
			if (tokens.length > 0)
				return  Integer.parseInt(tokens[0]);
		}
		throw new ParseException("mismatch", 0);
	}

	public Iterator<String> iterator() {
		return lines.iterator();
	}

	public int log_available() {
		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_TINY:
			if (stored_flight == 0)
				return 1;
			return 0;
		default:
			if (flight_log_max <= 0)
				return 1;
			int	log_space = storage_size - storage_erase_unit;
			int	log_used = stored_flight * flight_log_max;

			if (log_used >= log_space)
				return 0;
			return (log_space - log_used) / flight_log_max;
		}
	}

	int[] parse_version(String v) {
		String[] parts = v.split("\\.");
		int r[] = new int[parts.length];

		for (int i = 0; i < parts.length; i++) {
			try {
				r[i] = AltosLib.fromdec(parts[i]);
			} catch (NumberFormatException n) {
				r[i] = 0;
			}
		}

		return r;
	}
	
	public int compare_version(String other) {
		int[]	me = parse_version(version);
		int[]	them = parse_version(other);

		int	l = Math.min(me.length, them.length);

		for (int i = 0; i < l; i++) {
			int	d = me[i] - them[i];
			if (d != 0)
				return d;
		}
		if (me.length > l)
			return 1;
		if (them.length > l)
			return -1;
		return 0;
	}

	public AltosConfigData(AltosLink link) throws InterruptedException, TimeoutException {
		link.printf("c s\nf\nl\nv\n");
		lines = new LinkedList<String>();
		radio_setting = 0;
		radio_frequency = 0;
		stored_flight = 0;
		for (;;) {
			String line = link.get_reply();
			if (line == null)
				throw new TimeoutException();
			if (line.contains("Syntax error"))
				continue;
			lines.add(line);
			try { serial = get_int(line, "serial-number"); } catch (Exception e) {}
			try { log_format = get_int(line, "log-format"); } catch (Exception e) {}
			try { main_deploy = get_int(line, "Main deploy:"); } catch (Exception e) {}
			try { apogee_delay = get_int(line, "Apogee delay:"); } catch (Exception e) {}
			try { radio_channel = get_int(line, "Radio channel:"); } catch (Exception e) {}
			try { radio_setting = get_int(line, "Radio setting:"); } catch (Exception e) {}
			try {
				radio_frequency = get_int(line, "Frequency:");
				if (radio_frequency < 0)
					radio_frequency = 434550;
			} catch (Exception e) {}
			try {
				if (line.startsWith("Accel cal")) {
					String[] bits = line.split("\\s+");
					if (bits.length >= 6) {
						accel_cal_plus = Integer.parseInt(bits[3]);
						accel_cal_minus = Integer.parseInt(bits[5]);
					}
				}
			} catch (Exception e) {}
			try { radio_calibration = get_int(line, "Radio cal:"); } catch (Exception e) {}
			try { flight_log_max = get_int(line, "Max flight log:"); } catch (Exception e) {}
			try { ignite_mode = get_int(line, "Ignite mode:"); } catch (Exception e) {}
			try { callsign = get_string(line, "Callsign:"); } catch (Exception e) {}
			try { version = get_string(line,"software-version"); } catch (Exception e) {}
			try { product = get_string(line,"product"); } catch (Exception e) {}
			try { manufacturer = get_string(line,"manufacturer"); } catch (Exception e) {}

			try { get_int(line, "flight"); stored_flight++; }  catch (Exception e) {}
			try { storage_size = get_int(line, "Storage size:"); } catch (Exception e) {}
			try { storage_erase_unit = get_int(line, "Storage erase unit"); } catch (Exception e) {}

			/* signals the end of the version info */
			if (line.startsWith("software-version"))
				break;
		}
	}

}
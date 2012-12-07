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
	public int	serial;
	public int	flight;
	public int	log_format;
	public String	version;

	/* Strings returned */
	public LinkedList<String>	lines;

	/* Config information */
	/* HAS_FLIGHT*/
	public int	main_deploy;
	public int	apogee_delay;
	public int	apogee_lockout;
	
	/* HAS_RADIO */
	public int	radio_frequency;
	public String	callsign;
	public int	radio_enable;
	public int	radio_calibration;
	/* Old HAS_RADIO values */
	public int	radio_channel;
	public int	radio_setting;

	/* HAS_ACCEL */
	public int	accel_cal_plus, accel_cal_minus;
	public int	pad_orientation;

	/* HAS_LOG */
	public int	flight_log_max;

	/* HAS_IGNITE */
	public int	ignite_mode;

	/* HAS_AES */
	public String	aes_key;

	/* AO_PYRO_NUM */
	public AltosPyro[]	pyros;
	public int		npyro;
	public int		pyro;

	/* Storage info replies */
	public int	storage_size;
	public int	storage_erase_unit;

	/* Log listing replies */
	public int	stored_flight;


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
		case AltosLib.AO_LOG_FORMAT_TELEMETRY:
		case AltosLib.AO_LOG_FORMAT_TELESCIENCE:
			return 1;
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

	public void reset() {
		lines = new LinkedList<String>();

		serial = -1;
		radio_setting = 0;
		radio_frequency = 0;
		pyros = null;
		npyro = 0;
		pyro = 0;
	}
	
	public void parse_line(String line) {
		lines.add(line);
		/* Version replies */
		try { manufacturer = get_string(line, "manufacturer"); } catch (Exception e) {}
		try { product = get_string(line, "product"); } catch (Exception e) {}
		try { serial = get_int(line, "serial-number"); } catch (Exception e) {}
		try { flight = get_int(line, "current-flight"); } catch (Exception e) {}
		try { log_format = get_int(line, "log-format"); } catch (Exception e) {}
		try { version = get_string(line, "software-version"); } catch (Exception e) {}

		/* Version also contains MS5607 info, which we ignore here */

		/* Config show replies */

		/* HAS_FLIGHT */
		try { main_deploy = get_int(line, "Main deploy:"); } catch (Exception e) {}
		try { apogee_delay = get_int(line, "Apogee delay:"); } catch (Exception e) {}
		try { apogee_lockout = get_int(line, "Apogee lockout:"); } catch (Exception e) {}

		/* HAS_RADIO */
		try {
			radio_frequency = get_int(line, "Frequency:");
			if (radio_frequency < 0)
				radio_frequency = 434550;
		} catch (Exception e) {}
		try { callsign = get_string(line, "Callsign:"); } catch (Exception e) {}
		try { radio_enable = get_int(line, "Radio enable:"); } catch (Exception e) {}
		try { radio_calibration = get_int(line, "Radio cal:"); } catch (Exception e) {}

		/* Old HAS_RADIO values */
		try { radio_channel = get_int(line, "Radio channel:"); } catch (Exception e) {}
		try { radio_setting = get_int(line, "Radio setting:"); } catch (Exception e) {}

		/* HAS_ACCEL */
		try {
			if (line.startsWith("Accel cal")) {
				String[] bits = line.split("\\s+");
				if (bits.length >= 6) {
					accel_cal_plus = Integer.parseInt(bits[3]);
					accel_cal_minus = Integer.parseInt(bits[5]);
				}
			}
		} catch (Exception e) {}
		try { pad_orientation = get_int(line, "Pad orientation:"); } catch (Exception e) {}

		/* HAS_LOG */
		try { flight_log_max = get_int(line, "Max flight log:"); } catch (Exception e) {}

		/* HAS_IGNITE */
		try { ignite_mode = get_int(line, "Ignite mode:"); } catch (Exception e) {}

		/* HAS_AES */
		try { aes_key = get_string(line, "AES key:"); } catch (Exception e) {}

		/* AO_PYRO_NUM */
		try {
			npyro = get_int(line, "Pyro-count:");
			pyros = new AltosPyro[npyro];
			pyro = 0;
		} catch (Exception e) {}
		if (npyro > 0) {
			try {
				AltosPyro p = new AltosPyro(pyro, line);
				if (pyro < npyro - 1)
					pyros[pyro++] = p;
			} catch (Exception e) {}
		}
		
		/* Storage info replies */
		try { storage_size = get_int(line, "Storage size:"); } catch (Exception e) {}
		try { storage_erase_unit = get_int(line, "Storage erase unit"); } catch (Exception e) {}

		/* Log listing replies */
		try { get_int(line, "flight"); stored_flight++; }  catch (Exception e) {}
	}

	public AltosConfigData() {
		this.reset();
	}

	private void read_link(AltosLink link, String finished) throws InterruptedException, TimeoutException {
		for (;;) {
			String line = link.get_reply();
			if (line == null)
				throw new TimeoutException();
			if (line.contains("Syntax error"))
				continue;
			this.parse_line(line);

			/* signals the end of the version info */
			if (line.startsWith(finished))
				break;
		}
	}

	public AltosConfigData(AltosLink link) throws InterruptedException, TimeoutException {
		this.reset();
		link.printf("c s\nf\nv\n");
		read_link(link, "software-version");
		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_TELEMETRY:
		case AltosLib.AO_LOG_FORMAT_TELESCIENCE:
			break;
		default:
			link.printf("l\n");
			read_link(link, "done");
		}
	}

}
/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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
import java.text.*;

public class AltosEepromHeader extends AltosEeprom {

	public int	cmd;
	public String	data;
	public int	config_a, config_b;
	public boolean	last;
	public boolean	valid;

	public int record_length () { return 0; }

	/* XXX pull rest of config data to state */
	public void update_state(AltosState state) {
		switch (cmd) {
		case AltosLib.AO_LOG_CONFIG_VERSION:
			break;
		case AltosLib.AO_LOG_MAIN_DEPLOY:
			break;
		case AltosLib.AO_LOG_APOGEE_DELAY:
			break;
		case AltosLib.AO_LOG_RADIO_CHANNEL:
			break;
		case AltosLib.AO_LOG_CALLSIGN:
			state.set_callsign(data);
			break;
		case AltosLib.AO_LOG_ACCEL_CAL:
			state.set_accel_g(config_a, config_b);
			break;
		case AltosLib.AO_LOG_RADIO_CAL:
			break;
		case AltosLib.AO_LOG_MANUFACTURER:
			break;
		case AltosLib.AO_LOG_PRODUCT:
			state.product = data;
			break;
		case AltosLib.AO_LOG_LOG_FORMAT:
			state.set_log_format(config_a);
			break;
		case AltosLib.AO_LOG_SERIAL_NUMBER:
			state.set_serial(config_a);
			break;
		case AltosLib.AO_LOG_BARO_RESERVED:
			state.make_baro();
			state.baro.reserved = config_a;
			break;
		case AltosLib.AO_LOG_BARO_SENS:
			state.make_baro();
			state.baro.sens = config_a;
			break;
		case AltosLib.AO_LOG_BARO_OFF:
			state.make_baro();
			state.baro.off = config_a;
			break;
		case AltosLib.AO_LOG_BARO_TCS:
			state.make_baro();
			state.baro.tcs = config_a;
			break;
		case AltosLib.AO_LOG_BARO_TCO:
			state.make_baro();
			state.baro.tco = config_a;
			break;
		case AltosLib.AO_LOG_BARO_TREF:
			state.make_baro();
			state.baro.tref = config_a;
			break;
		case AltosLib.AO_LOG_BARO_TEMPSENS:
			state.make_baro();
			state.baro.tempsens = config_a;
			break;
		case AltosLib.AO_LOG_BARO_CRC:
			state.make_baro();
			state.baro.crc = config_a;
			break;
		case AltosLib.AO_LOG_SOFTWARE_VERSION:
			state.set_firmware_version(data);
			break;
		}
	}

	public void write(PrintStream out) {
		switch (cmd) {
		case AltosLib.AO_LOG_CONFIG_VERSION:
			out.printf("# Config version: %s\n", data);
			break;
		case AltosLib.AO_LOG_MAIN_DEPLOY:
			out.printf("# Main deploy: %s\n", config_a);
			break;
		case AltosLib.AO_LOG_APOGEE_DELAY:
			out.printf("# Apogee delay: %s\n", config_a);
			break;
		case AltosLib.AO_LOG_RADIO_CHANNEL:
			out.printf("# Radio channel: %s\n", config_a);
			break;
		case AltosLib.AO_LOG_CALLSIGN:
			out.printf("# Callsign: %s\n", data);
			break;
		case AltosLib.AO_LOG_ACCEL_CAL:
			out.printf ("# Accel cal: %d %d\n", config_a, config_b);
			break;
		case AltosLib.AO_LOG_RADIO_CAL:
			out.printf ("# Radio cal: %d\n", config_a);
			break;
		case AltosLib.AO_LOG_MAX_FLIGHT_LOG:
			out.printf ("# Max flight log: %d\n", config_a);
			break;
		case AltosLib.AO_LOG_MANUFACTURER:
			out.printf ("# Manufacturer: %s\n", data);
			break;
		case AltosLib.AO_LOG_PRODUCT:
			out.printf ("# Product: %s\n", data);
			break;
		case AltosLib.AO_LOG_SERIAL_NUMBER:
			out.printf ("# Serial number: %d\n", config_a);
			break;
		case AltosLib.AO_LOG_SOFTWARE_VERSION:
			out.printf ("# Software version: %s\n", data);
			break;
		case AltosLib.AO_LOG_BARO_RESERVED:
			out.printf ("# Baro reserved: %d\n", config_a);
			break;
		case AltosLib.AO_LOG_BARO_SENS:
			out.printf ("# Baro sens: %d\n", config_a);
			break;
		case AltosLib.AO_LOG_BARO_OFF:
			out.printf ("# Baro off: %d\n", config_a);
			break;
		case AltosLib.AO_LOG_BARO_TCS:
			out.printf ("# Baro tcs: %d\n", config_a);
			break;
		case AltosLib.AO_LOG_BARO_TCO:
			out.printf ("# Baro tco: %d\n", config_a);
			break;
		case AltosLib.AO_LOG_BARO_TREF:
			out.printf ("# Baro tref: %d\n", config_a);
			break;
		case AltosLib.AO_LOG_BARO_TEMPSENS:
			out.printf ("# Baro tempsens: %d\n", config_a);
			break;
		case AltosLib.AO_LOG_BARO_CRC:
			out.printf ("# Baro crc: %d\n", config_a);
			break;
		}
	}

	public AltosEepromHeader (String[] tokens) {
		last = false;
		valid = true;
		try {
			if (tokens[0].equals("Config") && tokens[1].equals("version:")) {
				cmd = AltosLib.AO_LOG_CONFIG_VERSION;
				data = tokens[2];
			} else if (tokens[0].equals("Main") && tokens[1].equals("deploy:")) {
				cmd = AltosLib.AO_LOG_MAIN_DEPLOY;
				config_a = Integer.parseInt(tokens[2]);
			} else if (tokens[0].equals("Apogee") && tokens[1].equals("delay:")) {
				cmd = AltosLib.AO_LOG_APOGEE_DELAY;
				config_a = Integer.parseInt(tokens[2]);
			} else if (tokens[0].equals("Radio") && tokens[1].equals("channel:")) {
				cmd = AltosLib.AO_LOG_RADIO_CHANNEL;
				config_a = Integer.parseInt(tokens[2]);
			} else if (tokens[0].equals("Callsign:")) {
				cmd = AltosLib.AO_LOG_CALLSIGN;
				data = tokens[1].replaceAll("\"","");
			} else if (tokens[0].equals("Accel") && tokens[1].equals("cal")) {
				cmd = AltosLib.AO_LOG_ACCEL_CAL;
				config_a = Integer.parseInt(tokens[3]);
				config_b = Integer.parseInt(tokens[5]);
			} else if (tokens[0].equals("Radio") && tokens[1].equals("cal:")) {
				cmd = AltosLib.AO_LOG_RADIO_CAL;
				config_a = Integer.parseInt(tokens[2]);
			} else if (tokens[0].equals("Max") && tokens[1].equals("flight") && tokens[2].equals("log:")) {
				cmd = AltosLib.AO_LOG_MAX_FLIGHT_LOG;
				config_a = Integer.parseInt(tokens[3]);
			} else if (tokens[0].equals("manufacturer")) {
				cmd = AltosLib.AO_LOG_MANUFACTURER;
				data = tokens[1];
			} else if (tokens[0].equals("product")) {
				cmd = AltosLib.AO_LOG_PRODUCT;
				data = tokens[1];
			} else if (tokens[0].equals("serial-number")) {
				cmd = AltosLib.AO_LOG_SERIAL_NUMBER;
				config_a = Integer.parseInt(tokens[1]);
			} else if (tokens[0].equals("log-format")) {
				cmd = AltosLib.AO_LOG_LOG_FORMAT;
				config_a = Integer.parseInt(tokens[1]);
			} else if (tokens[0].equals("software-version")) {
				cmd = AltosLib.AO_LOG_SOFTWARE_VERSION;
				data = tokens[1];
				last = true;
			} else if (tokens[0].equals("ms5607")) {
				if (tokens[1].equals("reserved:")) {
					cmd = AltosLib.AO_LOG_BARO_RESERVED;
					config_a = Integer.parseInt(tokens[2]);
				} else if (tokens[1].equals("sens:")) {
					cmd = AltosLib.AO_LOG_BARO_SENS;
					config_a = Integer.parseInt(tokens[2]);
				} else if (tokens[1].equals("off:")) {
					cmd = AltosLib.AO_LOG_BARO_OFF;
					config_a = Integer.parseInt(tokens[2]);
				} else if (tokens[1].equals("tcs:")) {
					cmd = AltosLib.AO_LOG_BARO_TCS;
					config_a = Integer.parseInt(tokens[2]);
				} else if (tokens[1].equals("tco:")) {
					cmd = AltosLib.AO_LOG_BARO_TCO;
					config_a = Integer.parseInt(tokens[2]);
				} else if (tokens[1].equals("tref:")) {
					cmd = AltosLib.AO_LOG_BARO_TREF;
					config_a = Integer.parseInt(tokens[2]);
				} else if (tokens[1].equals("tempsens:")) {
					cmd = AltosLib.AO_LOG_BARO_TEMPSENS;
					config_a = Integer.parseInt(tokens[2]);
				} else if (tokens[1].equals("crc:")) {
					cmd = AltosLib.AO_LOG_BARO_CRC;
					config_a = Integer.parseInt(tokens[2]);
				} else {
					cmd = AltosLib.AO_LOG_INVALID;
					data = tokens[2];
				}
			} else
				valid = false;
		} catch (Exception e) {
			valid = false;
		}
	}

	static public LinkedList<AltosEeprom> read(FileInputStream input) {
		LinkedList<AltosEeprom> headers = new LinkedList<AltosEeprom>();

		for (;;) {
			try {
				String line = AltosLib.gets(input);
				if (line == null)
					break;
				AltosEepromHeader header = new AltosEepromHeader(line);
				headers.add(header);
				if (header.last)
					break;
			} catch (IOException ie) {
				break;
			}
		}

		return headers;
	}

	static public void write (PrintStream out, LinkedList<AltosEepromHeader> headers) {
		out.printf("# Comments\n");
		for (AltosEepromHeader header : headers) {
			header.write(out);
		}

	}

	public AltosEepromHeader (String line) {
		this(line.split("\\s+"));
	}
}

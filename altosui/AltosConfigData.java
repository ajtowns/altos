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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.table.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.util.concurrent.*;

import libaltosJNI.*;

public class AltosConfigData implements Iterable<String> {

	/* Version information */
	String	manufacturer;
	String	product;
	String	version;
	int	serial;

	/* Strings returned */
	LinkedList<String>	lines;

	/* Config information */
	int	config_major;
	int	config_minor;
	int	main_deploy;
	int	apogee_delay;
	int	radio_channel;
	String	callsign;
	int	accel_cal_plus, accel_cal_minus;
	int	radio_calibration;
	int	flight_log_max;
	int	ignite_mode;


	static String get_string(String line, String label) throws  ParseException {
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

	static int get_int(String line, String label) throws NumberFormatException, ParseException {
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

	public AltosConfigData(AltosSerial serial_line) throws InterruptedException, TimeoutException {
		serial_line.printf("c s\nv\n");
		lines = new LinkedList<String>();
		for (;;) {
			String line = serial_line.get_reply(5000);
			if (line == null)
				throw new TimeoutException();
			if (line.contains("Syntax error"))
				continue;
			lines.add(line);
			try { serial = get_int(line, "serial-number"); } catch (Exception e) {}
			try { main_deploy = get_int(line, "Main deploy:"); } catch (Exception e) {}
			try { apogee_delay = get_int(line, "Apogee delay:"); } catch (Exception e) {}
			try { radio_channel = get_int(line, "Radio channel:"); } catch (Exception e) {}
			try { radio_calibration = get_int(line, "Radio cal:"); } catch (Exception e) {}
			try { flight_log_max = get_int(line, "Max flight log:"); } catch (Exception e) {}
			try { ignite_mode = get_int(line, "Ignite mode:"); } catch (Exception e) {}
			try { callsign = get_string(line, "Callsign:"); } catch (Exception e) {}
			try { version = get_string(line,"software-version"); } catch (Exception e) {}
			try { product = get_string(line,"product"); } catch (Exception e) {}

			/* signals the end of the version info */
			if (line.startsWith("software-version"))
				break;
		}
	}

}
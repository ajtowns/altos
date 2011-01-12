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

class AltosEepromFlight {
	int	flight;
	int	start;
	int	end;

	public AltosEepromFlight(int in_flight, int in_start, int in_end) {
		flight = in_flight;
		start = in_start;
		end = in_end;
	}
}

public class AltosEepromList extends ArrayList<AltosEepromLog> {
	AltosConfigData	config_data;

	public AltosEepromList (AltosSerial serial_line, boolean remote) throws IOException, InterruptedException, TimeoutException {
		try {
			if (remote)
				serial_line.start_remote();
			config_data = new AltosConfigData (serial_line);
			if (config_data.serial == 0)
				throw new IOException("no serial number found");

			ArrayList<AltosEepromFlight> flights = new ArrayList<AltosEepromFlight>();
			if (config_data.flight_log_max != 0) {
				serial_line.printf("l\n");
				for (;;) {
					String line = serial_line.get_reply(5000);
					if (line == null)
						throw new TimeoutException();
					if (line.contains("done"))
						break;
					if (line.contains("Syntax"))
						continue;
					String[] tokens = line.split("\\s+");
					System.out.printf("got line %s (%d tokens)\n", line, tokens.length);
					if (tokens.length < 6)
						break;

					int	flight = -1, start = -1, end = -1;
					try {
						if (tokens[0].equals("flight"))
							flight = AltosParse.parse_int(tokens[1]);
						if (tokens[2].equals("start"))
							start = AltosParse.parse_hex(tokens[3]);
						if (tokens[4].equals("end"))
							end = AltosParse.parse_hex(tokens[5]);
						System.out.printf("parsed flight %d %x %x\n", flight, start, end);
						if (flight > 0 && start >= 0 && end > 0)
							flights.add(new AltosEepromFlight(flight, start, end));
					} catch (ParseException pe) { System.out.printf("Parse error %s\n", pe.toString()); }
				}
			} else {
				flights.add(new AltosEepromFlight(0, 0, 0xfff));
			}
			for (AltosEepromFlight flight : flights) {
				System.out.printf("Scanning flight %d %x %x\n", flight.flight, flight.start, flight.end);
				add(new AltosEepromLog(serial_line, config_data.serial,
						       flight.start, flight.end));
			}
		} finally {
			if (remote)
				serial_line.stop_remote();
			serial_line.flush_output();
		}
		for (int i = 0; i < size(); i++) {
			AltosEepromLog	l = get(i);
			System.out.printf("Found flight %d at %x - %x\n", l.flight, l.start_block, l.end_block);
		}
	}
}
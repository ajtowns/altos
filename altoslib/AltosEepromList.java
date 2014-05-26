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

package org.altusmetrum.altoslib_4;

import java.io.*;
import java.util.*;
import java.text.*;
import java.util.concurrent.*;

/*
 * Temporary structure to hold the list of stored flights;
 * each of these will be queried in turn to generate more
 * complete information
 */

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

/*
 * Construct a list of flights available in a connected device
 */

public class AltosEepromList extends ArrayList<AltosEepromLog> {
	public AltosConfigData	config_data;

	public AltosEepromList (AltosLink link, boolean remote)
		throws IOException, InterruptedException, TimeoutException
	{
		try {
			if (remote)
				link.start_remote();
			config_data = new AltosConfigData (link);
//			if (config_data.serial == 0)
//				throw new IOException("no serial number found");

			ArrayList<AltosEepromFlight> flights = new ArrayList<AltosEepromFlight>();

			if (config_data.flight_log_max != 0 || config_data.log_format != 0) {

				/* Devices with newer firmware will support the 'l'
				 * command which will list the region of storage
				 * occupied by each available flight
				 */
				link.printf("l\n");
				for (;;) {
					String line = link.get_reply(5000);
					if (line == null)
						throw new TimeoutException();
					if (line.contains("done"))
						break;
					if (line.contains("Syntax"))
						continue;
					String[] tokens = line.split("\\s+");
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
						if (flight > 0 && start >= 0 && end > 0)
							flights.add(new AltosEepromFlight(flight, start, end));
					} catch (ParseException pe) { System.out.printf("Parse error %s\n", pe.toString()); }
				}
			} else {

				/* Older devices will hold only a single
				 * flight. This also assumes that any older
				 * device will have a 1MB flash device
				 */
				flights.add(new AltosEepromFlight(0, 0, 0xfff));
			}

			/* With the list of flights collected, collect more complete
			 * information on them by reading the first block or two of
			 * data. This will add GPS coordinates and a date. For older
			 * firmware, this will also extract the flight number.
			 */
			for (AltosEepromFlight flight : flights) {
				add(new AltosEepromLog(config_data, link,
						       flight.flight, flight.start, flight.end));
			}
		} finally {
			if (remote)
				link.stop_remote();
			link.flush_output();
		}
	}
}
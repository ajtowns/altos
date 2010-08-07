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

import java.lang.*;
import java.io.*;
import altosui.AltosRecord;

public class AltosCSV {
	File		name;
	PrintStream	out;
	boolean		header_written;

	static final int ALTOS_CSV_VERSION = 1;

	/* Version 1 format:
	 *
	 * General info
	 *	version number
	 *	serial number
	 *	flight number
	 *	callsign
	 *	time (seconds since boost)
	 *
	 * Flight status
	 *	state
	 *	state name
	 *
	 * Basic sensors
	 *	acceleration (m/s²)
	 *	pressure (mBar)
	 *	altitude (m)
	 *	accelerometer speed (m/s)
	 *	barometer speed (m/s)
	 *	temp (°C)
	 *	battery (V)
	 *	drogue (V)
	 *	main (V)
	 *
	 * GPS data
	 *	connected (1/0)
	 *	locked (1/0)
	 *	nsat (used for solution)
	 *	latitude (°)
	 *	longitude (°)
	 *	altitude (m)
	 *	year (e.g. 2010)
	 *	month (1-12)
	 *	day (1-31)
	 *	hour (0-23)
	 *	minute (0-59)
	 *	second (0-59)
	 *
	 * GPS Sat data
	 *	hdop
	 *	C/N0 data for all 32 valid SDIDs
	 */

	void write_general_header() {
		out.printf("version serial flight call time");
	}

	void write_general(AltosRecord record) {
		out.printf("%s,%d,%d,%s,%d",
			   record.version, record.serial, record.flight, record.callsign, record.tick);
	}

	void write_flight_header() {
		out.printf("state state_name");
	}

	void write_flight(AltosRecord record) {
		out.printf("%d,%s", record.state, record.state());
	}

	void write_header() {
		out.printf("# "); write_general_header();
		out.printf(" "); write_flight_header();
		out.printf ("\n");
	}

	public void write(AltosRecord record) {
		if (!header_written) {
			write_header();
			header_written = true;
		}
		write_general(record); out.printf(",");
		write_flight(record);
		out.printf ("\n");
	}

	public PrintStream out() {
		return out;
	}

	public AltosCSV(File in_name) throws FileNotFoundException {
		name = in_name;
		out = new PrintStream(name);
	}
}

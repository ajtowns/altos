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
import java.text.*;
import java.util.*;

import altosui.AltosRecord;
import altosui.AltosReader;

public class AltosCSV {
	File			name;
	PrintStream		out;
	boolean			header_written;
	boolean			seen_boost;
	int			boost_tick;
	LinkedList<AltosRecord>	pad_records;
	AltosState		state;

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
	 *	height (m)
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
		out.printf("%s,%d,%d,%s,%8.2f",
			   record.version, record.serial, record.flight, record.callsign,
			   (double) (record.tick - boost_tick) / 100.0);
	}

	void write_flight_header() {
		out.printf("state state_name");
	}

	void write_flight(AltosRecord record) {
		out.printf("%d,%8s", record.state, record.state());
	}

	void write_basic_header() {
		out.printf("acceleration pressure altitude height accel_speed baro_speed temperature battery_voltage drogue_voltage main_voltage");
	}

	void write_basic(AltosRecord record) {
		out.printf("%8.2f,%10.2f,%8.2f,%8.2f,%8.2f,%8.2f,%5.1f,%5.2f,%5.2f,%5.2f",
			   record.acceleration(),
			   record.pressure(),
			   record.altitude(),
			   record.height(),
			   record.accel_speed(),
			   state.baro_speed,
			   record.temperature(),
			   record.battery_voltage(),
			   record.drogue_voltage(),
			   record.main_voltage());
	}

	void write_gps_header() {
		out.printf("connected locked nsat latitude longitude altitude year month day hour minute second");
	}

	void write_gps(AltosRecord record) {
		AltosGPS	gps = record.gps;
		if (gps == null)
			gps = new AltosGPS();

		out.printf("%2d,%2d,%3d,%12.7f,%12.7f,%6d,%5d,%3d,%3d,%3d,%3d,%3d",
			   gps.connected?1:0,
			   gps.locked?1:0,
			   gps.nsat,
			   gps.lat,
			   gps.lon,
			   gps.alt,
			   gps.year,
			   gps.month,
			   gps.day,
			   gps.hour,
			   gps.minute,
			   gps.second);
	}

	void write_header() {
		out.printf("# "); write_general_header();
		out.printf(" "); write_flight_header();
		out.printf(" "); write_basic_header();
		out.printf(" "); write_gps_header();
		out.printf ("\n");
	}

	void write_one(AltosRecord record) {
		state = new AltosState(record, state);
		write_general(record); out.printf(",");
		write_flight(record); out.printf(",");
		write_basic(record); out.printf(",");
		write_gps(record);
		out.printf ("\n");
	}

	void flush_pad() {
		while (!pad_records.isEmpty()) {
			write_one (pad_records.remove());
		}
	}

	public void write(AltosRecord record) {
		if (!header_written) {
			write_header();
			header_written = true;
		}
		if (!seen_boost) {
			if (record.state >= Altos.ao_flight_boost) {
				seen_boost = true;
				boost_tick = record.tick;
				flush_pad();
			}
		}
		if (seen_boost)
			write_one(record);
		else
			pad_records.add(record);
	}

	public PrintStream out() {
		return out;
	}

	public void close() {
		if (!pad_records.isEmpty()) {
			boost_tick = pad_records.element().tick;
			flush_pad();
		}
		out.close();
	}

	public void write(AltosReader reader) {
		AltosRecord	record;

		reader.write_comments(out());
		try {
			for (;;) {
				record = reader.read();
				if (record == null)
					break;
				write(record);
			}
		} catch (IOException ie) {
		} catch (ParseException pe) {
		}
	}

	public AltosCSV(File in_name) throws FileNotFoundException {
		name = in_name;
		out = new PrintStream(name);
		pad_records = new LinkedList<AltosRecord>();
	}
}

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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.table.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.util.concurrent.LinkedBlockingQueue;

import altosui.AltosRecord;
import altosui.AltosState;
import altosui.AltosDeviceDialog;
import altosui.AltosPreferences;
import altosui.AltosLog;
import altosui.AltosVoice;
import altosui.AltosEepromMonitor;

/*
 * AltosRecords with an index field so they can be sorted by tick while preserving
 * the original ordering for elements with matching ticks
 */
class AltosOrderedRecord extends AltosEepromRecord implements Comparable<AltosOrderedRecord> {

	int	index;

	public AltosOrderedRecord(String line, int in_index, int prev_tick)
		throws ParseException {
		super(line);
		int new_tick = tick | (prev_tick & ~0xffff);
		if (new_tick < prev_tick) {
			if (prev_tick - new_tick > 0x8000)
				new_tick += 0x10000;
		}
		tick = new_tick;
		index = in_index;
	}

	public int compareTo(AltosOrderedRecord o) {
		int	tick_diff = tick - o.tick;
		if (tick_diff != 0)
			return tick_diff;
		return index - o.index;
	}
}

public class AltosEepromReader extends AltosReader {

	static final int	seen_flight = 1;
	static final int	seen_sensor = 2;
	static final int	seen_temp_volt = 4;
	static final int	seen_deploy = 8;
	static final int	seen_gps_time = 16;
	static final int	seen_gps_lat = 32;
	static final int	seen_gps_lon = 64;

	static final int	seen_basic = seen_flight|seen_sensor|seen_temp_volt|seen_deploy;

	AltosRecord		state;
	AltosOrderedRecord	record;

	TreeSet<AltosOrderedRecord>	records;

	Iterator<AltosOrderedRecord>			record_iterator;

	int			seen;

	int			index;

	boolean			last_reported;

	double			ground_pres;
	double			ground_accel;

	int			n_pad_samples;

	int			gps_tick;

	boolean			saw_boost;

	int			boost_tick;

	public AltosRecord read() throws IOException, ParseException {
		for (;;) {
			if (record == null) {
				if (!record_iterator.hasNext()) {
					if (last_reported)
						return null;
					last_reported = true;
					return state;
				}
				record = record_iterator.next();

				if ((seen & seen_basic) == seen_basic && record.tick != state.tick) {
					AltosRecord r = new AltosRecord(state);
					r.time = (r.tick - boost_tick) / 100.0;
					return r;
				}
			}

			state.tick = record.tick;
			switch (record.cmd) {
			case Altos.AO_LOG_FLIGHT:
				state.ground_accel = record.a;
				state.flight = record.b;
				seen |= seen_flight;
				break;
			case Altos.AO_LOG_SENSOR:
				state.accel = record.a;
				state.pres = record.b;
				if (state.state < Altos.ao_flight_boost) {
					n_pad_samples++;
					ground_pres += state.pres;
					state.ground_pres = (int) (ground_pres / n_pad_samples);
					state.flight_pres = state.ground_pres;
					System.out.printf("ground pressure %d altitude %f\n",
							  record.b, state.altitude());
					ground_accel += state.accel;
					state.ground_accel = (int) (ground_accel / n_pad_samples);
					state.flight_accel = state.ground_accel;
				} else {
					state.flight_pres = (state.flight_pres * 15 + state.pres) / 16;
					state.flight_accel = (state.flight_accel * 15 + state.accel) / 16;
					state.flight_vel += (state.accel_plus_g - state.accel);
				}
				seen |= seen_sensor;
				break;
			case Altos.AO_LOG_TEMP_VOLT:
				state.temp = record.a;
				state.batt = record.b;
				seen |= seen_temp_volt;
				break;
			case Altos.AO_LOG_DEPLOY:
				state.drogue = record.a;
				state.main = record.b;
				seen |= seen_deploy;
				break;
			case Altos.AO_LOG_STATE:
				System.out.printf("state %d\n", record.a);
				state.state = record.a;
				break;
			case Altos.AO_LOG_GPS_TIME:
				gps_tick = state.tick;
				state.gps = new AltosGPS();
				state.gps.hour = (record.a & 0xff);
				state.gps.minute = (record.a >> 8);
				state.gps.second = (record.b & 0xff);
				int flags = (record.b >> 8);
				state.gps.connected = (flags & Altos.AO_GPS_RUNNING) != 0;
				state.gps.locked = (flags & Altos.AO_GPS_VALID) != 0;
				state.gps.date_valid = (flags & Altos.AO_GPS_DATE_VALID) != 0;
				state.gps.nsat = (flags & Altos.AO_GPS_NUM_SAT_MASK) >>
					Altos.AO_GPS_NUM_SAT_SHIFT;
				break;
			case Altos.AO_LOG_GPS_LAT:
				int lat32 = record.a | (record.b << 16);
				state.gps.lat = (double) lat32 / 1e7;
				break;
			case Altos.AO_LOG_GPS_LON:
				int lon32 = record.a | (record.b << 16);
				state.gps.lon = (double) lon32 / 1e7;
				break;
			case Altos.AO_LOG_GPS_ALT:
				state.gps.alt = record.a;
				break;
			case Altos.AO_LOG_GPS_SAT:
				if (state.tick == gps_tick) {
					int svid = record.a;
					int c_n0 = record.b >> 8;
					state.gps.add_sat(svid, c_n0);
				}
				break;
			case Altos.AO_LOG_GPS_DATE:
				state.gps.year = record.a & 0xff;
				state.gps.month = record.a >> 8;
				state.gps.day = record.b & 0xff;
				break;

			case Altos.AO_LOG_CONFIG_VERSION:
				break;
			case Altos.AO_LOG_MAIN_DEPLOY:
				break;
			case Altos.AO_LOG_APOGEE_DELAY:
				break;
			case Altos.AO_LOG_RADIO_CHANNEL:
				break;
			case Altos.AO_LOG_CALLSIGN:
				state.callsign = record.data;
				break;
			case Altos.AO_LOG_ACCEL_CAL:
				state.accel_plus_g = record.a;
				state.accel_minus_g = record.b;
				break;
			case Altos.AO_LOG_RADIO_CAL:
				break;
			case Altos.AO_LOG_MANUFACTURER:
				break;
			case Altos.AO_LOG_PRODUCT:
				break;
			case Altos.AO_LOG_SERIAL_NUMBER:
				break;
			case Altos.AO_LOG_SOFTWARE_VERSION:
				break;
			}
			record = null;
		}
	}

	public void write_comments(PrintStream out) {
		Iterator<AltosOrderedRecord>	iterator = records.iterator();
		while (iterator.hasNext()) {
			AltosOrderedRecord	record = iterator.next();
			switch (record.cmd) {
			case Altos.AO_LOG_CONFIG_VERSION:
				out.printf("# Config version: %s\n", record.data);
				break;
			case Altos.AO_LOG_MAIN_DEPLOY:
				out.printf("# Main deploy: %s\n", record.a);
				break;
			case Altos.AO_LOG_APOGEE_DELAY:
				out.printf("# Apogee delay: %s\n", record.a);
				break;
			case Altos.AO_LOG_RADIO_CHANNEL:
				out.printf("# Radio channel: %s\n", record.a);
				break;
			case Altos.AO_LOG_CALLSIGN:
				out.printf("# Callsign: %s\n", record.data);
				break;
			case Altos.AO_LOG_ACCEL_CAL:
				out.printf ("# Accel cal: %d %d\n", record.a, record.b);
				break;
			case Altos.AO_LOG_RADIO_CAL:
				out.printf ("# Radio cal: %d %d\n", record.a);
				break;
			case Altos.AO_LOG_MANUFACTURER:
				out.printf ("# Manufacturer: %s\n", record.data);
				break;
			case Altos.AO_LOG_PRODUCT:
				out.printf ("# Product: %s\n", record.data);
				break;
			case Altos.AO_LOG_SERIAL_NUMBER:
				out.printf ("# Serial number: %d\n", record.a);
				break;
			case Altos.AO_LOG_SOFTWARE_VERSION:
				out.printf ("# Software version: %s\n", record.data);
				break;
			}
		}
	}

	/*
	 * Read the whole file, dumping records into a RB tree so
	 * we can enumerate them in time order -- the eeprom data
	 * are sometimes out of order with GPS data getting timestamps
	 * matching the first packet out of the GPS unit but not
	 * written until the final GPS packet has been received.
	 */
	public AltosEepromReader (FileInputStream input) {
		state = new AltosRecord();
		state.state = Altos.ao_flight_pad;
		state.accel_plus_g = 15758;
		state.accel_minus_g = 16294;
		seen = 0;
		records = new TreeSet<AltosOrderedRecord>();

		int index = 0;
		int tick = 0;

		try {
			for (;;) {
				String line = AltosRecord.gets(input);
				if (line == null)
					break;
				AltosOrderedRecord record = new AltosOrderedRecord(line, index++, tick);
				if (record == null)
					break;
				tick = record.tick;
				if (!saw_boost && record.cmd == Altos.AO_LOG_STATE &&
				    record.a == Altos.ao_flight_boost)
				{
					saw_boost = true;
					boost_tick = state.tick;
				}
				records.add(record);
			}
		} catch (IOException io) {
		} catch (ParseException pe) {
		}
		record_iterator = records.iterator();
		try {
			input.close();
		} catch (IOException ie) {
		}
	}
}

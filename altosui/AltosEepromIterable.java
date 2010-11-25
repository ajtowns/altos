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

/*
 * AltosRecords with an index field so they can be sorted by tick while preserving
 * the original ordering for elements with matching ticks
 */
class AltosOrderedRecord extends AltosEepromRecord implements Comparable<AltosOrderedRecord> {

	public int	index;

	public AltosOrderedRecord(String line, int in_index, int prev_tick, boolean prev_tick_valid)
		throws ParseException {
		super(line);
		if (prev_tick_valid) {
			tick |= (prev_tick & ~0xffff);
			if (tick < prev_tick) {
				if (prev_tick - tick > 0x8000)
					tick += 0x10000;
			} else {
				if (tick - prev_tick > 0x8000)
					tick -= 0x10000;
			}
		}
		index = in_index;
	}

	public AltosOrderedRecord(int in_cmd, int in_tick, int in_a, int in_b, int in_index) {
		super(in_cmd, in_tick, in_a, in_b);
		index = in_index;
	}

	public int compareTo(AltosOrderedRecord o) {
		int	tick_diff = tick - o.tick;
		if (tick_diff != 0)
			return tick_diff;
		return index - o.index;
	}
}

public class AltosEepromIterable extends AltosRecordIterable {

	static final int	seen_flight = 1;
	static final int	seen_sensor = 2;
	static final int	seen_temp_volt = 4;
	static final int	seen_deploy = 8;
	static final int	seen_gps_time = 16;
	static final int	seen_gps_lat = 32;
	static final int	seen_gps_lon = 64;

	static final int	seen_basic = seen_flight|seen_sensor|seen_temp_volt|seen_deploy;

	AltosEepromRecord	flight_record;
	AltosEepromRecord	gps_date_record;

	TreeSet<AltosOrderedRecord>	records;

	LinkedList<AltosRecord>	list;

	class EepromState {
		int	seen;
		int	n_pad_samples;
		double	ground_pres;
		int	gps_tick;
		int	boost_tick;

		EepromState() {
			seen = 0;
			n_pad_samples = 0;
			ground_pres = 0.0;
			gps_tick = 0;
		}
	}

	void update_state(AltosRecord state, AltosEepromRecord record, EepromState eeprom) {
		state.tick = record.tick;
		switch (record.cmd) {
		case Altos.AO_LOG_FLIGHT:
			eeprom.seen |= seen_flight;
			state.ground_accel = record.a;
			state.flight_accel = record.a;
			state.flight = record.b;
			eeprom.boost_tick = record.tick;
			break;
		case Altos.AO_LOG_SENSOR:
			state.accel = record.a;
			state.pres = record.b;
			if (state.state < Altos.ao_flight_boost) {
				eeprom.n_pad_samples++;
				eeprom.ground_pres += state.pres;
				state.ground_pres = (int) (eeprom.ground_pres / eeprom.n_pad_samples);
				state.flight_pres = state.ground_pres;
			} else {
				state.flight_pres = (state.flight_pres * 15 + state.pres) / 16;
				state.flight_accel = (state.flight_accel * 15 + state.accel) / 16;
				state.flight_vel += (state.accel_plus_g - state.accel);
			}
			eeprom.seen |= seen_sensor;
			break;
		case Altos.AO_LOG_TEMP_VOLT:
			state.temp = record.a;
			state.batt = record.b;
			eeprom.seen |= seen_temp_volt;
			break;
		case Altos.AO_LOG_DEPLOY:
			state.drogue = record.a;
			state.main = record.b;
			eeprom.seen |= seen_deploy;
			break;
		case Altos.AO_LOG_STATE:
			state.state = record.a;
			break;
		case Altos.AO_LOG_GPS_TIME:
			eeprom.gps_tick = state.tick;
			AltosGPS old = state.gps;
			state.gps = new AltosGPS();

			/* GPS date doesn't get repeated through the file */
			if (old != null) {
				state.gps.year = old.year;
				state.gps.month = old.month;
				state.gps.day = old.day;
			}
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
			if (state.tick == eeprom.gps_tick) {
				int svid = record.a;
				int c_n0 = record.b >> 8;
				state.gps.add_sat(svid, c_n0);
			}
			break;
		case Altos.AO_LOG_GPS_DATE:
			state.gps.year = (record.a & 0xff) + 2000;
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
			state.serial = record.a;
			break;
		case Altos.AO_LOG_SOFTWARE_VERSION:
			break;
		}
	}

	LinkedList<AltosRecord> make_list() {
		LinkedList<AltosRecord>		list = new LinkedList<AltosRecord>();
		Iterator<AltosOrderedRecord>	iterator = records.iterator();
		AltosOrderedRecord		record = null;
		AltosRecord			state = new AltosRecord();
		boolean				last_reported = false;
		EepromState			eeprom = new EepromState();

		state.state = Altos.ao_flight_pad;
		state.accel_plus_g = 15758;
		state.accel_minus_g = 16294;

		/* Pull in static data from the flight and gps_date records */
		if (flight_record != null)
			update_state(state, flight_record, eeprom);
		if (gps_date_record != null)
			update_state(state, gps_date_record, eeprom);

		while (iterator.hasNext()) {
			record = iterator.next();
			if ((eeprom.seen & seen_basic) == seen_basic && record.tick != state.tick) {
				AltosRecord r = new AltosRecord(state);
				r.time = (r.tick - eeprom.boost_tick) / 100.0;
				list.add(r);
			}
			update_state(state, record, eeprom);
		}
		AltosRecord r = new AltosRecord(state);
		r.time = (r.tick - eeprom.boost_tick) / 100.0;
		list.add(r);
		return list;
	}

	public Iterator<AltosRecord> iterator() {
		if (list == null)
			list = make_list();
		return list.iterator();
	}

	public void write_comments(PrintStream out) {
		Iterator<AltosOrderedRecord>	iterator = records.iterator();
		out.printf("# Comments\n");
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
				out.printf ("# Radio cal: %d\n", record.a);
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
	 * Given an AO_LOG_GPS_TIME record with correct time, and one
	 * missing time, rewrite the missing time values with the good
	 * ones, assuming that the difference between them is 'diff' seconds
	 */
	void update_time(AltosOrderedRecord good, AltosOrderedRecord bad) {

		int diff = (bad.tick - good.tick + 50) / 100;

		int hour = (good.a & 0xff);
		int minute = (good.a >> 8);
		int second = (good.b & 0xff);
		int flags = (good.b >> 8);
		int seconds = hour * 3600 + minute * 60 + second;

		/* Make sure this looks like a good GPS value */
		if ((flags & Altos.AO_GPS_NUM_SAT_MASK) >> Altos.AO_GPS_NUM_SAT_SHIFT < 4)
			flags = (flags & ~Altos.AO_GPS_NUM_SAT_MASK) | (4 << Altos.AO_GPS_NUM_SAT_SHIFT);
		flags |= Altos.AO_GPS_RUNNING;
		flags |= Altos.AO_GPS_VALID;

		int new_seconds = seconds + diff;
		if (new_seconds < 0)
			new_seconds += 24 * 3600;
		int new_second = (new_seconds % 60);
		int new_minutes = (new_seconds / 60);
		int new_minute = (new_minutes % 60);
		int new_hours = (new_minutes / 60);
		int new_hour = (new_hours % 24);

		bad.a = new_hour + (new_minute << 8);
		bad.b = new_second + (flags << 8);
	}

	/*
	 * Read the whole file, dumping records into a RB tree so
	 * we can enumerate them in time order -- the eeprom data
	 * are sometimes out of order with GPS data getting timestamps
	 * matching the first packet out of the GPS unit but not
	 * written until the final GPS packet has been received.
	 */
	public AltosEepromIterable (FileInputStream input) {
		records = new TreeSet<AltosOrderedRecord>();

		AltosOrderedRecord last_gps_time = null;

		int index = 0;
		int prev_tick = 0;
		boolean prev_tick_valid = false;
		boolean missing_time = false;

		try {
			for (;;) {
				String line = AltosRecord.gets(input);
				if (line == null)
					break;
				AltosOrderedRecord record = new AltosOrderedRecord(line, index++, prev_tick, prev_tick_valid);
				if (record == null)
					break;
				if (record.cmd == Altos.AO_LOG_INVALID)
					continue;
				prev_tick = record.tick;
				if (record.cmd < Altos.AO_LOG_CONFIG_VERSION)
					prev_tick_valid = true;
				if (record.cmd == Altos.AO_LOG_FLIGHT) {
					flight_record = record;
					continue;
				}

				/* Two firmware bugs caused the loss of some GPS data.
				 * The flight date would never be recorded, and often
				 * the flight time would get overwritten by another
				 * record. Detect the loss of the GPS date and fix up the
				 * missing time records
				 */
				if (record.cmd == Altos.AO_LOG_GPS_DATE) {
					gps_date_record = record;
					continue;
				}

				/* go back and fix up any missing time values */
				if (record.cmd == Altos.AO_LOG_GPS_TIME) {
					last_gps_time = record;
					if (missing_time) {
						Iterator<AltosOrderedRecord> iterator = records.iterator();
						while (iterator.hasNext()) {
							AltosOrderedRecord old = iterator.next();
							if (old.cmd == Altos.AO_LOG_GPS_TIME &&
							    old.a == -1 && old.b == -1)
							{
								update_time(record, old);
							}
						}
						missing_time = false;
					}
				}

				if (record.cmd == Altos.AO_LOG_GPS_LAT) {
					if (last_gps_time == null || last_gps_time.tick != record.tick) {
						AltosOrderedRecord add_gps_time = new AltosOrderedRecord(Altos.AO_LOG_GPS_TIME,
													 record.tick,
													 -1, -1, index-1);
						if (last_gps_time != null)
							update_time(last_gps_time, add_gps_time);
						else
							missing_time = true;

						records.add(add_gps_time);
						record.index = index++;
					}
				}
				records.add(record);

				/* Bail after reading the 'landed' record; we're all done */
				if (record.cmd == Altos.AO_LOG_STATE &&
				    record.a == Altos.ao_flight_landed)
					break;
			}
		} catch (IOException io) {
		} catch (ParseException pe) {
		}
		try {
			input.close();
		} catch (IOException ie) {
		}
	}
}

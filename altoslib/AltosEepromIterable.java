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

package org.altusmetrum.AltosLib;

import java.io.*;
import java.util.*;
import java.text.*;

public class AltosEepromIterable extends AltosRecordIterable {

	static final int	seen_basic = AltosRecord.seen_flight|AltosRecord.seen_sensor;

	boolean			has_accel;
	boolean			has_gps;
	boolean			has_ignite;

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
		int	sensor_tick;

		EepromState() {
			seen = 0;
			n_pad_samples = 0;
			ground_pres = 0.0;
			gps_tick = 0;
		}
	}

	void update_state(AltosRecordTM state, AltosEepromRecord record, EepromState eeprom) {
		state.tick = record.tick;
		switch (record.cmd) {
		case AltosLib.AO_LOG_FLIGHT:
			eeprom.seen |= AltosRecord.seen_flight;
			state.ground_accel = record.a;
			state.flight_accel = record.a;
			state.flight = record.b;
			eeprom.boost_tick = record.tick;
			break;
		case AltosLib.AO_LOG_SENSOR:
			state.accel = record.a;
			state.pres = record.b;
			if (state.state < AltosLib.ao_flight_boost) {
				eeprom.n_pad_samples++;
				eeprom.ground_pres += state.pres;
				state.ground_pres = (int) (eeprom.ground_pres / eeprom.n_pad_samples);
				state.flight_pres = state.ground_pres;
			} else {
				state.flight_pres = (state.flight_pres * 15 + state.pres) / 16;
			}
			state.flight_accel = (state.flight_accel * 15 + state.accel) / 16;
			if ((eeprom.seen & AltosRecord.seen_sensor) == 0)
				eeprom.sensor_tick = record.tick - 1;
			state.flight_vel += (state.accel_plus_g - state.accel) * (record.tick - eeprom.sensor_tick);
			eeprom.seen |= AltosRecord.seen_sensor;
			eeprom.sensor_tick = record.tick;
			has_accel = true;
			break;
		case AltosLib.AO_LOG_PRESSURE:
			state.pres = record.b;
			state.flight_pres = state.pres;
			if (eeprom.n_pad_samples == 0) {
				eeprom.n_pad_samples++;
				state.ground_pres = state.pres;
			}
			eeprom.seen |= AltosRecord.seen_sensor;
			break;
		case AltosLib.AO_LOG_TEMP_VOLT:
			state.temp = record.a;
			state.batt = record.b;
			eeprom.seen |= AltosRecord.seen_temp_volt;
			break;
		case AltosLib.AO_LOG_DEPLOY:
			state.drogue = record.a;
			state.main = record.b;
			eeprom.seen |= AltosRecord.seen_deploy;
			has_ignite = true;
			break;
		case AltosLib.AO_LOG_STATE:
			state.state = record.a;
			break;
		case AltosLib.AO_LOG_GPS_TIME:
			eeprom.gps_tick = state.tick;
			eeprom.seen |= AltosRecord.seen_gps_time;
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
			state.gps.connected = (flags & AltosLib.AO_GPS_RUNNING) != 0;
			state.gps.locked = (flags & AltosLib.AO_GPS_VALID) != 0;
			state.gps.nsat = (flags & AltosLib.AO_GPS_NUM_SAT_MASK) >>
				AltosLib.AO_GPS_NUM_SAT_SHIFT;
			state.new_gps = true;
			has_gps = true;
			break;
		case AltosLib.AO_LOG_GPS_LAT:
			eeprom.seen |= AltosRecord.seen_gps_lat;
			int lat32 = record.a | (record.b << 16);
			state.gps.lat = (double) lat32 / 1e7;
			break;
		case AltosLib.AO_LOG_GPS_LON:
			eeprom.seen |= AltosRecord.seen_gps_lon;
			int lon32 = record.a | (record.b << 16);
			state.gps.lon = (double) lon32 / 1e7;
			break;
		case AltosLib.AO_LOG_GPS_ALT:
			state.gps.alt = record.a;
			break;
		case AltosLib.AO_LOG_GPS_SAT:
			if (state.tick == eeprom.gps_tick) {
				int svid = record.a;
				int c_n0 = record.b >> 8;
				state.gps.add_sat(svid, c_n0);
			}
			break;
		case AltosLib.AO_LOG_GPS_DATE:
			state.gps.year = (record.a & 0xff) + 2000;
			state.gps.month = record.a >> 8;
			state.gps.day = record.b & 0xff;
			break;

		case AltosLib.AO_LOG_CONFIG_VERSION:
			break;
		case AltosLib.AO_LOG_MAIN_DEPLOY:
			break;
		case AltosLib.AO_LOG_APOGEE_DELAY:
			break;
		case AltosLib.AO_LOG_RADIO_CHANNEL:
			break;
		case AltosLib.AO_LOG_CALLSIGN:
			state.callsign = record.data;
			break;
		case AltosLib.AO_LOG_ACCEL_CAL:
			state.accel_plus_g = record.a;
			state.accel_minus_g = record.b;
			break;
		case AltosLib.AO_LOG_RADIO_CAL:
			break;
		case AltosLib.AO_LOG_MANUFACTURER:
			break;
		case AltosLib.AO_LOG_PRODUCT:
			break;
		case AltosLib.AO_LOG_SERIAL_NUMBER:
			state.serial = record.a;
			break;
		case AltosLib.AO_LOG_SOFTWARE_VERSION:
			break;
		}
		state.seen |= eeprom.seen;
	}

	LinkedList<AltosRecord> make_list() {
		LinkedList<AltosRecord>		list = new LinkedList<AltosRecord>();
		Iterator<AltosOrderedRecord>	iterator = records.iterator();
		AltosOrderedRecord		record = null;
		AltosRecordTM			state = new AltosRecordTM();
		//boolean				last_reported = false;
		EepromState			eeprom = new EepromState();

		state.state = AltosLib.ao_flight_pad;
		state.accel_plus_g = 15758;
		state.accel_minus_g = 16294;
		state.flight_vel = 0;

		/* Pull in static data from the flight and gps_date records */
		if (flight_record != null)
			update_state(state, flight_record, eeprom);
		if (gps_date_record != null)
			update_state(state, gps_date_record, eeprom);

		while (iterator.hasNext()) {
			record = iterator.next();
			if ((eeprom.seen & seen_basic) == seen_basic && record.tick != state.tick) {
				AltosRecordTM r = state.clone();
				r.time = (r.tick - eeprom.boost_tick) / 100.0;
				list.add(r);
			}
			update_state(state, record, eeprom);
		}
		AltosRecordTM r = state.clone();
		r.time = (r.tick - eeprom.boost_tick) / 100.0;
		list.add(r);
	return list;
	}

	public Iterator<AltosRecord> iterator() {
		if (list == null)
			list = make_list();
		return list.iterator();
	}

	public boolean has_gps() { return has_gps; }
	public boolean has_accel() { return has_accel; }
	public boolean has_ignite() { return has_ignite; }

	public void write_comments(PrintStream out) {
		Iterator<AltosOrderedRecord>	iterator = records.iterator();
		out.printf("# Comments\n");
		while (iterator.hasNext()) {
			AltosOrderedRecord	record = iterator.next();
			switch (record.cmd) {
			case AltosLib.AO_LOG_CONFIG_VERSION:
				out.printf("# Config version: %s\n", record.data);
				break;
			case AltosLib.AO_LOG_MAIN_DEPLOY:
				out.printf("# Main deploy: %s\n", record.a);
				break;
			case AltosLib.AO_LOG_APOGEE_DELAY:
				out.printf("# Apogee delay: %s\n", record.a);
				break;
			case AltosLib.AO_LOG_RADIO_CHANNEL:
				out.printf("# Radio channel: %s\n", record.a);
				break;
			case AltosLib.AO_LOG_CALLSIGN:
				out.printf("# Callsign: %s\n", record.data);
				break;
			case AltosLib.AO_LOG_ACCEL_CAL:
				out.printf ("# Accel cal: %d %d\n", record.a, record.b);
				break;
			case AltosLib.AO_LOG_RADIO_CAL:
				out.printf ("# Radio cal: %d\n", record.a);
				break;
			case AltosLib.AO_LOG_MAX_FLIGHT_LOG:
				out.printf ("# Max flight log: %d\n", record.a);
				break;
			case AltosLib.AO_LOG_MANUFACTURER:
				out.printf ("# Manufacturer: %s\n", record.data);
				break;
			case AltosLib.AO_LOG_PRODUCT:
				out.printf ("# Product: %s\n", record.data);
				break;
			case AltosLib.AO_LOG_SERIAL_NUMBER:
				out.printf ("# Serial number: %d\n", record.a);
				break;
			case AltosLib.AO_LOG_SOFTWARE_VERSION:
				out.printf ("# Software version: %s\n", record.data);
				break;
			case AltosLib.AO_LOG_BARO_RESERVED:
				out.printf ("# Baro reserved: %d\n", record.a);
				break;
			case AltosLib.AO_LOG_BARO_SENS:
				out.printf ("# Baro sens: %d\n", record.a);
				break;
			case AltosLib.AO_LOG_BARO_OFF:
				out.printf ("# Baro off: %d\n", record.a);
				break;
			case AltosLib.AO_LOG_BARO_TCS:
				out.printf ("# Baro tcs: %d\n", record.a);
				break;
			case AltosLib.AO_LOG_BARO_TCO:
				out.printf ("# Baro tco: %d\n", record.a);
				break;
			case AltosLib.AO_LOG_BARO_TREF:
				out.printf ("# Baro tref: %d\n", record.a);
				break;
			case AltosLib.AO_LOG_BARO_TEMPSENS:
				out.printf ("# Baro tempsens: %d\n", record.a);
				break;
			case AltosLib.AO_LOG_BARO_CRC:
				out.printf ("# Baro crc: %d\n", record.a);
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
		if ((flags & AltosLib.AO_GPS_NUM_SAT_MASK) >> AltosLib.AO_GPS_NUM_SAT_SHIFT < 4)
			flags = (flags & ~AltosLib.AO_GPS_NUM_SAT_MASK) | (4 << AltosLib.AO_GPS_NUM_SAT_SHIFT);
		flags |= AltosLib.AO_GPS_RUNNING;
		flags |= AltosLib.AO_GPS_VALID;

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
				String line = AltosLib.gets(input);
				if (line == null)
					break;
				AltosOrderedRecord record = new AltosOrderedRecord(line, index++, prev_tick, prev_tick_valid);
				if (record.cmd == AltosLib.AO_LOG_INVALID)
					continue;
				prev_tick = record.tick;
				if (record.cmd < AltosLib.AO_LOG_CONFIG_VERSION)
					prev_tick_valid = true;
				if (record.cmd == AltosLib.AO_LOG_FLIGHT) {
					flight_record = record;
					continue;
				}

				/* Two firmware bugs caused the loss of some GPS data.
				 * The flight date would never be recorded, and often
				 * the flight time would get overwritten by another
				 * record. Detect the loss of the GPS date and fix up the
				 * missing time records
				 */
				if (record.cmd == AltosLib.AO_LOG_GPS_DATE) {
					gps_date_record = record;
					continue;
				}

				/* go back and fix up any missing time values */
				if (record.cmd == AltosLib.AO_LOG_GPS_TIME) {
					last_gps_time = record;
					if (missing_time) {
						Iterator<AltosOrderedRecord> iterator = records.iterator();
						while (iterator.hasNext()) {
							AltosOrderedRecord old = iterator.next();
							if (old.cmd == AltosLib.AO_LOG_GPS_TIME &&
							    old.a == -1 && old.b == -1)
							{
								update_time(record, old);
							}
						}
						missing_time = false;
					}
				}

				if (record.cmd == AltosLib.AO_LOG_GPS_LAT) {
					if (last_gps_time == null || last_gps_time.tick != record.tick) {
						AltosOrderedRecord add_gps_time = new AltosOrderedRecord(AltosLib.AO_LOG_GPS_TIME,
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
				if (record.cmd == AltosLib.AO_LOG_STATE &&
				    record.a == AltosLib.ao_flight_landed)
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

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

package org.altusmetrum.altoslib_1;

import java.io.*;
import java.util.*;
import java.text.*;

public class AltosEepromMegaIterable extends AltosRecordIterable {

	static final int	seen_flight = 1;
	static final int	seen_sensor = 2;
	static final int	seen_temp_volt = 4;
	static final int	seen_deploy = 8;
	static final int	seen_gps_time = 16;
	static final int	seen_gps_lat = 32;
	static final int	seen_gps_lon = 64;

	static final int	seen_basic = seen_flight|seen_sensor;

	boolean			has_accel;
	boolean			has_gps;
	boolean			has_ignite;

	AltosEepromMega	flight_record;
	AltosEepromMega	gps_date_record;

	TreeSet<AltosOrderedMegaRecord>	records;

	AltosMs5607		baro;

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

	void update_state(AltosRecordMM state, AltosEepromMega record, EepromState eeprom) {
		state.tick = record.tick;
		switch (record.cmd) {
		case AltosLib.AO_LOG_FLIGHT:
			eeprom.seen |= seen_flight;
			state.ground_accel = record.ground_accel();
			state.flight_accel = record.ground_accel();
			state.ground_pres = baro.set(record.ground_pres(), record.ground_temp());
			state.flight_pres = state.ground_pres;
			state.flight = record.data16(0);
			eeprom.boost_tick = record.tick;
			break;
		case AltosLib.AO_LOG_SENSOR:
			state.accel = record.accel();
			baro.set(record.pres(), record.temp());
			state.pres = baro.pa;
			state.temp = baro.cc;
			state.imu = new AltosIMU();
			state.imu.accel_x = record.accel_x();
			state.imu.accel_y = record.accel_y();
			state.imu.accel_z = record.accel_z();
			state.imu.gyro_x = record.gyro_x();
			state.imu.gyro_y = record.gyro_y();
			state.imu.gyro_z = record.gyro_z();
			state.mag = new AltosMag();
			state.mag.x = record.mag_x();
			state.mag.y = record.mag_y();
			state.mag.z = record.mag_z();
			if (state.state < AltosLib.ao_flight_boost) {
				eeprom.n_pad_samples++;
				eeprom.ground_pres += state.pres;
				state.ground_pres = (int) (eeprom.ground_pres / eeprom.n_pad_samples);
				state.flight_pres = state.ground_pres;
			} else {
				state.flight_pres = (state.flight_pres * 15 + state.pres) / 16;
			}
			state.flight_accel = (state.flight_accel * 15 + state.accel) / 16;
			if ((eeprom.seen & seen_sensor) == 0)
				eeprom.sensor_tick = record.tick - 1;
			state.flight_vel += (state.accel_plus_g - state.accel) * (record.tick - eeprom.sensor_tick);
			eeprom.seen |= seen_sensor;
			eeprom.sensor_tick = record.tick;
			has_accel = true;
			break;
		case AltosLib.AO_LOG_TEMP_VOLT:
			state.v_batt = record.v_batt();
			state.v_pyro = record.v_pbatt();
			for (int i = 0; i < record.nsense(); i++)
				state.sense[i] = record.sense(i);
			eeprom.seen |= seen_temp_volt;
			break;
		case AltosLib.AO_LOG_STATE:
			state.state = record.state();
			break;
		case AltosLib.AO_LOG_GPS_TIME:
			eeprom.gps_tick = state.tick;
			state.gps = new AltosGPS();

			state.gps.lat = record.latitude() / 1e7;
			state.gps.lon = record.longitude() / 1e7;
			state.gps.alt = record.altitude();
			state.gps.year = record.year() + 2000;
			state.gps.month = record.month();
			state.gps.day = record.day();

			state.gps.hour = record.hour();
			state.gps.minute = record.minute();
			state.gps.second = record.second();

			int flags = record.flags();
			state.gps.connected = (flags & AltosLib.AO_GPS_RUNNING) != 0;
			state.gps.locked = (flags & AltosLib.AO_GPS_VALID) != 0;
			state.gps.nsat = (flags & AltosLib.AO_GPS_NUM_SAT_MASK) >>
				AltosLib.AO_GPS_NUM_SAT_SHIFT;
			state.gps_sequence++;
			has_gps = true;
			eeprom.seen |= seen_gps_time | seen_gps_lat | seen_gps_lon;
			break;
		case AltosLib.AO_LOG_GPS_SAT:
			if (state.tick == eeprom.gps_tick) {
				int nsat = record.nsat();
				for (int i = 0; i < nsat; i++)
					state.gps.add_sat(record.svid(i), record.c_n(i));
			}
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
			state.accel_plus_g = record.config_a;
			state.accel_minus_g = record.config_b;
			break;
		case AltosLib.AO_LOG_RADIO_CAL:
			break;
		case AltosLib.AO_LOG_MANUFACTURER:
			break;
		case AltosLib.AO_LOG_PRODUCT:
			break;
		case AltosLib.AO_LOG_SERIAL_NUMBER:
			state.serial = record.config_a;
			break;
		case AltosLib.AO_LOG_SOFTWARE_VERSION:
			break;
		case AltosLib.AO_LOG_BARO_RESERVED:
			baro.reserved = record.config_a;
			break;
		case AltosLib.AO_LOG_BARO_SENS:
			baro.sens =record.config_a;
			break;
		case AltosLib.AO_LOG_BARO_OFF:
			baro.off =record.config_a;
			break;
		case AltosLib.AO_LOG_BARO_TCS:
			baro.tcs =record.config_a;
			break;
		case AltosLib.AO_LOG_BARO_TCO:
			baro.tco =record.config_a;
			break;
		case AltosLib.AO_LOG_BARO_TREF:
			baro.tref =record.config_a;
			break;
		case AltosLib.AO_LOG_BARO_TEMPSENS:
			baro.tempsens =record.config_a;
			break;
		case AltosLib.AO_LOG_BARO_CRC:
			baro.crc =record.config_a;
			break;
		}
		state.seen |= eeprom.seen;
	}

	LinkedList<AltosRecord> make_list() {
		LinkedList<AltosRecord>		list = new LinkedList<AltosRecord>();
		Iterator<AltosOrderedMegaRecord>	iterator = records.iterator();
		AltosOrderedMegaRecord		record = null;
		AltosRecordMM			state = new AltosRecordMM();
		//boolean				last_reported = false;
		EepromState			eeprom = new EepromState();

		state.state = AltosLib.ao_flight_pad;
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
				AltosRecordMM r = state.clone();
				r.time = (r.tick - eeprom.boost_tick) / 100.0;
				list.add(r);
			}
			update_state(state, record, eeprom);
		}
		AltosRecordMM r = state.clone();
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
		Iterator<AltosOrderedMegaRecord>	iterator = records.iterator();
		out.printf("# Comments\n");
		while (iterator.hasNext()) {
			AltosOrderedMegaRecord	record = iterator.next();
			switch (record.cmd) {
			case AltosLib.AO_LOG_CONFIG_VERSION:
				out.printf("# Config version: %s\n", record.data);
				break;
			case AltosLib.AO_LOG_MAIN_DEPLOY:
				out.printf("# Main deploy: %s\n", record.config_a);
				break;
			case AltosLib.AO_LOG_APOGEE_DELAY:
				out.printf("# Apogee delay: %s\n", record.config_a);
				break;
			case AltosLib.AO_LOG_RADIO_CHANNEL:
				out.printf("# Radio channel: %s\n", record.config_a);
				break;
			case AltosLib.AO_LOG_CALLSIGN:
				out.printf("# Callsign: %s\n", record.data);
				break;
			case AltosLib.AO_LOG_ACCEL_CAL:
				out.printf ("# Accel cal: %d %d\n", record.config_a, record.config_b);
				break;
			case AltosLib.AO_LOG_RADIO_CAL:
				out.printf ("# Radio cal: %d\n", record.config_a);
				break;
			case AltosLib.AO_LOG_MAX_FLIGHT_LOG:
				out.printf ("# Max flight log: %d\n", record.config_a);
				break;
			case AltosLib.AO_LOG_MANUFACTURER:
				out.printf ("# Manufacturer: %s\n", record.data);
				break;
			case AltosLib.AO_LOG_PRODUCT:
				out.printf ("# Product: %s\n", record.data);
				break;
			case AltosLib.AO_LOG_SERIAL_NUMBER:
				out.printf ("# Serial number: %d\n", record.config_a);
				break;
			case AltosLib.AO_LOG_SOFTWARE_VERSION:
				out.printf ("# Software version: %s\n", record.data);
				break;
			case AltosLib.AO_LOG_BARO_RESERVED:
				out.printf ("# Baro reserved: %d\n", record.config_a);
				break;
			case AltosLib.AO_LOG_BARO_SENS:
				out.printf ("# Baro sens: %d\n", record.config_a);
				break;
			case AltosLib.AO_LOG_BARO_OFF:
				out.printf ("# Baro off: %d\n", record.config_a);
				break;
			case AltosLib.AO_LOG_BARO_TCS:
				out.printf ("# Baro tcs: %d\n", record.config_a);
				break;
			case AltosLib.AO_LOG_BARO_TCO:
				out.printf ("# Baro tco: %d\n", record.config_a);
				break;
			case AltosLib.AO_LOG_BARO_TREF:
				out.printf ("# Baro tref: %d\n", record.config_a);
				break;
			case AltosLib.AO_LOG_BARO_TEMPSENS:
				out.printf ("# Baro tempsens: %d\n", record.config_a);
				break;
			case AltosLib.AO_LOG_BARO_CRC:
				out.printf ("# Baro crc: %d\n", record.config_a);
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
	public AltosEepromMegaIterable (FileInputStream input) {
		records = new TreeSet<AltosOrderedMegaRecord>();

		AltosOrderedMegaRecord last_gps_time = null;

		baro = new AltosMs5607();

		int index = 0;
		int prev_tick = 0;
		boolean prev_tick_valid = false;
		boolean missing_time = false;

		try {
			for (;;) {
				String line = AltosLib.gets(input);
				if (line == null)
					break;
				AltosOrderedMegaRecord record = new AltosOrderedMegaRecord(line, index++, prev_tick, prev_tick_valid);
				if (record.cmd == AltosLib.AO_LOG_INVALID)
					continue;
				prev_tick = record.tick;
				if (record.cmd < AltosLib.AO_LOG_CONFIG_VERSION)
					prev_tick_valid = true;
				if (record.cmd == AltosLib.AO_LOG_FLIGHT) {
					flight_record = record;
					continue;
				}

				records.add(record);

				/* Bail after reading the 'landed' record; we're all done */
				if (record.cmd == AltosLib.AO_LOG_STATE &&
				    record.state() == AltosLib.ao_flight_landed)
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

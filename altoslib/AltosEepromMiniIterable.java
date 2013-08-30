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

public class AltosEepromMiniIterable implements Iterable<AltosEepromMini> {

	static final int	seen_flight = 1;
	static final int	seen_sensor = 2;

	static final int	seen_basic = seen_flight|seen_sensor;

	boolean			has_accel;
	boolean			has_gps;
	boolean			has_ignite;

	AltosEepromMini	flight_record;

	TreeSet<AltosOrderedMiniRecord>	records;

	AltosMs5607		baro;

	LinkedList<AltosRecord>	list;

	class EepromState {
		int	seen;
		int	n_pad_samples;
		double	ground_pres;
		int	boost_tick;
		int	sensor_tick;

		EepromState() {
			seen = 0;
			n_pad_samples = 0;
			ground_pres = 0.0;
		}
	}

	void update_state(AltosRecordMini state, AltosEepromMini record, EepromState eeprom) {
		state.tick = record.tick;
		switch (record.cmd) {
		case AltosLib.AO_LOG_FLIGHT:
			eeprom.seen |= seen_flight;
			state.ground_pres = record.ground_pres();
			state.flight_pres = state.ground_pres;
			state.flight = record.data16(0);
			eeprom.boost_tick = record.tick;
			break;
		case AltosLib.AO_LOG_SENSOR:
			baro.set(record.pres(), record.temp());
			state.pres = baro.pa;
			state.temp = baro.cc;
			state.sense_m = record.sense_m();
			state.sense_a = record.sense_a();
			state.v_batt = record.v_batt();
			if (state.state < AltosLib.ao_flight_boost) {
				eeprom.n_pad_samples++;
				eeprom.ground_pres += state.pres;
				state.ground_pres = (int) (eeprom.ground_pres / eeprom.n_pad_samples);
				state.flight_pres = state.ground_pres;
			} else {
				state.flight_pres = (state.flight_pres * 15 + state.pres) / 16;
			}
			if ((eeprom.seen & seen_sensor) == 0)
				eeprom.sensor_tick = record.tick - 1;
			eeprom.seen |= seen_sensor;
			eeprom.sensor_tick = record.tick;
			break;
		case AltosLib.AO_LOG_STATE:
			state.state = record.state();
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
		Iterator<AltosOrderedMiniRecord> iterator = records.iterator();
		AltosOrderedMiniRecord		record = null;
		AltosRecordMini			state = new AltosRecordMini();
		//boolean			last_reported = false;
		EepromState			eeprom = new EepromState();

		state.state = AltosLib.ao_flight_pad;

		/* Pull in static data from the flight records */
		if (flight_record != null)
			update_state(state, flight_record, eeprom);

		while (iterator.hasNext()) {
			record = iterator.next();
			if ((eeprom.seen & seen_basic) == seen_basic && record.tick != state.tick) {
				AltosRecordMini r = state.clone();
				r.time = (r.tick - eeprom.boost_tick) / 100.0;
				list.add(r);
			}
			update_state(state, record, eeprom);
		}
		AltosRecordMini r = state.clone();
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
		Iterator<AltosOrderedMiniRecord>	iterator = records.iterator();
		out.printf("# Comments\n");
		while (iterator.hasNext()) {
			AltosOrderedMiniRecord	record = iterator.next();
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
	 * are sometimes out of order
	 */
	public AltosEepromMiniIterable (FileInputStream input) {
		records = new TreeSet<AltosOrderedMiniRecord>();

		AltosOrderedMiniRecord last_gps_time = null;

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
				AltosOrderedMiniRecord record = new AltosOrderedMiniRecord(line, index++, prev_tick, prev_tick_valid);
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

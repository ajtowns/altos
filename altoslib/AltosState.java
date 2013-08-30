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

/*
 * Track flight state from telemetry or eeprom data stream
 */

package org.altusmetrum.altoslib_1;

public class AltosState implements Cloneable {
	public AltosRecord record;

	public static final int set_position = 1;
	public static final int set_gps = 2;
	public static final int set_data = 4;

	public int set;

	/* derived data */

	public long  	report_time;

	public double	time;
	public double	prev_time;
	public double	time_change;
	public int	tick;
	public int	boost_tick;

	public int	state;
	public int	flight;
	public int	serial;
	public boolean	landed;
	public boolean	ascent;	/* going up? */
	public boolean	boost;	/* under power */
	public int	rssi;
	public int	status;

	public double	ground_altitude;
	public double	ground_pressure;
	public double	altitude;
	public double	height;
	public double	pressure;
	public double	acceleration;
	public double	battery_voltage;
	public double	pyro_voltage;
	public double	temperature;
	public double	apogee_voltage;
	public double	main_voltage;
	public double	speed;

	public double	prev_height;
	public double	prev_speed;
	public double	prev_acceleration;

	public double	max_height;
	public double	max_acceleration;
	public double	max_speed;

	public double	kalman_height, kalman_speed, kalman_acceleration;

	public AltosGPS	gps;
	public AltosGPS	temp_gps;
	public boolean	gps_pending;
	public int gps_sequence;

	public AltosIMU	imu;
	public AltosMag	mag;

	public static final int MIN_PAD_SAMPLES = 10;

	public int	npad;
	public int	gps_waiting;
	public boolean	gps_ready;

	public int	ngps;

	public AltosGreatCircle from_pad;
	public double	elevation;	/* from pad */
	public double	range;		/* total distance */

	public double	gps_height;

	public double pad_lat, pad_lon, pad_alt;

	public int	speak_tick;
	public double	speak_altitude;

	public String	callsign;
	public double	accel_plus_g;
	public double	accel_minus_g;
	public double	accel;
	public double	ground_accel;

	public int	log_format;

	public AltosMs5607	baro;

	public AltosRecordCompanion	companion;

	public double speed() {
		return speed;
	}

	public double max_speed() {
		return max_speed;
	}

	public void set_npad(int npad) {
		this.npad = npad;
		gps_waiting = MIN_PAD_SAMPLES - npad;
		if (this.gps_waiting < 0)
			gps_waiting = 0;
		gps_ready = gps_waiting == 0;
	}

	public void init() {
		record = null;

		set = 0;

		report_time = System.currentTimeMillis();
		time = AltosRecord.MISSING;
		time_change = AltosRecord.MISSING;
		prev_time = AltosRecord.MISSING;
		tick = AltosRecord.MISSING;
		boost_tick = AltosRecord.MISSING;
		state = AltosLib.ao_flight_invalid;
		flight = AltosRecord.MISSING;
		landed = false;
		boost = false;
		rssi = AltosRecord.MISSING;
		status = 0;

		ground_altitude = AltosRecord.MISSING;
		ground_pressure = AltosRecord.MISSING;
		altitude = AltosRecord.MISSING;
		height = AltosRecord.MISSING;
		pressure = AltosRecord.MISSING;
		acceleration = AltosRecord.MISSING;
		temperature = AltosRecord.MISSING;

		prev_height = AltosRecord.MISSING;
		prev_speed = AltosRecord.MISSING;
		prev_acceleration = AltosRecord.MISSING;

		battery_voltage = AltosRecord.MISSING;
		pyro_voltage = AltosRecord.MISSING;
		apogee_voltage = AltosRecord.MISSING;
		main_voltage = AltosRecord.MISSING;

		speed = AltosRecord.MISSING;

		kalman_height = AltosRecord.MISSING;
		kalman_speed = AltosRecord.MISSING;
		kalman_acceleration = AltosRecord.MISSING;

		max_speed = 0;
		max_height = 0;
		max_acceleration = 0;

		gps = null;
		temp_gps = null;
		gps_sequence = 0;
		gps_pending = false;

		imu = null;
		mag = null;

		set_npad(0);
		ngps = 0;

		from_pad = null;
		elevation = AltosRecord.MISSING;
		range = AltosRecord.MISSING;
		gps_height = AltosRecord.MISSING;

		pad_lat = AltosRecord.MISSING;
		pad_lon = AltosRecord.MISSING;
		pad_alt = AltosRecord.MISSING;

		speak_tick = AltosRecord.MISSING;
		speak_altitude = AltosRecord.MISSING;

		callsign = null;

		accel_plus_g = AltosRecord.MISSING;
		accel_minus_g = AltosRecord.MISSING;
		accel = AltosRecord.MISSING;
		ground_accel = AltosRecord.MISSING;
		log_format = AltosRecord.MISSING;
		serial = AltosRecord.MISSING;

		baro = null;
		companion = null;
	}

	void copy(AltosState old) {

		record = null;

		if (old == null) {
			init();
			return;
		}

		report_time = old.report_time;
		time = old.time;
		time_change = 0;
		tick = old.tick;
		boost_tick = old.boost_tick;

		state = old.state;
		flight = old.flight;
		landed = old.landed;
		ascent = old.ascent;
		boost = old.boost;
		rssi = old.rssi;
		status = old.status;
		
		set = 0;

		ground_altitude = old.ground_altitude;
		altitude = old.altitude;
		height = old.height;
		pressure = old.pressure;
		acceleration = old.acceleration;
		battery_voltage = old.battery_voltage;
		pyro_voltage = old.pyro_voltage;
		temperature = old.temperature;
		apogee_voltage = old.apogee_voltage;
		main_voltage = old.main_voltage;
		speed = old.speed;

		prev_height = old.height;
		prev_speed = old.speed;
		prev_acceleration = old.acceleration;
		prev_time = old.time;

		max_height = old.max_height;
		max_acceleration = old.max_acceleration;
		max_speed = old.max_speed;

		kalman_height = old.kalman_height;
		kalman_speed = old.kalman_speed;
		kalman_acceleration = old.kalman_acceleration;

		if (old.gps != null)
			gps = old.gps.clone();
		else
			gps = null;
		if (old.temp_gps != null)
			temp_gps = old.temp_gps.clone();
		else
			temp_gps = null;
		gps_sequence = old.gps_sequence;
		gps_pending = old.gps_pending;

		if (old.imu != null)
			imu = old.imu.clone();
		else
			imu = null;

		if (old.mag != null)
			mag = old.mag.clone();
		else
			mag = null;

		npad = old.npad;
		gps_waiting = old.gps_waiting;
		gps_ready = old.gps_ready;
		ngps = old.ngps;

		if (old.from_pad != null)
			from_pad = old.from_pad.clone();
		else
			from_pad = null;

		elevation = old.elevation;
		range = old.range;

		gps_height = old.gps_height;
		pad_lat = old.pad_lat;
		pad_lon = old.pad_lon;
		pad_alt = old.pad_alt;

		speak_tick = old.speak_tick;
		speak_altitude = old.speak_altitude;

		callsign = old.callsign;

		accel_plus_g = old.accel_plus_g;
		accel_minus_g = old.accel_minus_g;
		accel = old.accel;
		ground_accel = old.ground_accel;

		log_format = old.log_format;
		serial = old.serial;

		baro = old.baro;
		companion = old.companion;
	}

	double altitude() {
		if (altitude != AltosRecord.MISSING)
			return altitude;
		if (gps != null)
			return gps.alt;
		return AltosRecord.MISSING;
	}

	void update_vertical_pos() {

		double	alt = altitude();

		if (state == AltosLib.ao_flight_pad && alt != AltosRecord.MISSING && ground_pressure == AltosRecord.MISSING) {
			if (ground_altitude == AltosRecord.MISSING)
				ground_altitude = alt;
			else
				ground_altitude = (ground_altitude * 7 + alt) / 8;
		}

		if (kalman_height != AltosRecord.MISSING)
			height = kalman_height;
		else if (altitude != AltosRecord.MISSING && ground_altitude != AltosRecord.MISSING)
			height = altitude - ground_altitude;
		else
			height = AltosRecord.MISSING;

		if (height != AltosRecord.MISSING && height > max_height)
			max_height = height;

		update_speed();
	}

	double motion_filter_value() {
		return 1/ Math.exp(time_change/10.0);
	}

	void update_speed() {
		if (kalman_speed != AltosRecord.MISSING)
			speed = kalman_speed;
		else if (state != AltosLib.ao_flight_invalid &&
			 time_change != AltosRecord.MISSING)
		{
			if (ascent && acceleration != AltosRecord.MISSING)
			{
				if (prev_speed == AltosRecord.MISSING)
					speed = acceleration * time_change;
				else
					speed = prev_speed + acceleration * time_change;
			}
			else if (height != AltosRecord.MISSING &&
				 prev_height != AltosRecord.MISSING &&
				 time_change != 0)
			{
				double	new_speed = (height - prev_height) / time_change;

				if (prev_speed == AltosRecord.MISSING)
					speed = new_speed;
				else {
					double	filter = motion_filter_value();

					speed = prev_speed * filter + new_speed * (1-filter);
				}
			}
		}
		if (acceleration == AltosRecord.MISSING) {
			if (prev_speed != AltosRecord.MISSING && time_change != 0) {
				double	new_acceleration = (speed - prev_speed) / time_change;

				if (prev_acceleration == AltosRecord.MISSING)
					acceleration = new_acceleration;
				else {
					double filter = motion_filter_value();

					acceleration = prev_acceleration * filter + new_acceleration * (1-filter);
				}
			}
		}
		if (boost && speed != AltosRecord.MISSING && speed > max_speed)
			max_speed = speed;
	}
	
	void update_accel() {
		if (accel == AltosRecord.MISSING)
			return;
		if (ground_accel == AltosRecord.MISSING)
			return;
		if (accel_plus_g == AltosRecord.MISSING)
			return;
		if (accel_minus_g == AltosRecord.MISSING)
			return;

		double counts_per_g = (accel_minus_g - accel_plus_g) / 2.0;
		double counts_per_mss = counts_per_g / 9.80665;

		acceleration = (ground_accel - accel) / counts_per_mss;

		/* Only look at accelerometer data under boost */
		if (boost && acceleration != AltosRecord.MISSING && (max_acceleration == AltosRecord.MISSING || acceleration > max_acceleration))
			max_acceleration = acceleration;
		update_speed();
	}

	void update_time() {
		if (tick != AltosRecord.MISSING) {
			time = tick / 100.0;
			if (prev_time != AltosRecord.MISSING)
				time_change = time - prev_time;
		}
	}

	void update_gps() {
		elevation = 0;
		range = -1;
		gps_height = 0;

		if (gps == null)
			return;

		if (gps.locked && gps.nsat >= 4) {
			/* Track consecutive 'good' gps reports, waiting for 10 of them */
			if (state == AltosLib.ao_flight_pad) {
				set_npad(npad+1);
				if (pad_lat != AltosRecord.MISSING) {
					pad_lat = (pad_lat * 31 + gps.lat) / 32;
					pad_lon = (pad_lon * 31 + gps.lon) / 32;
					pad_alt = (pad_alt * 31 + gps.alt) / 32;
				}
			}
			if (pad_lat == AltosRecord.MISSING) {
				pad_lat = gps.lat;
				pad_lon = gps.lon;
				pad_alt = gps.alt;
			}
		}
		if (gps.lat != 0 && gps.lon != 0 &&
		    pad_lat != AltosRecord.MISSING &&
		    pad_lon != AltosRecord.MISSING)
		{
			double h = height;

			if (h == AltosRecord.MISSING)
				h = 0;
			from_pad = new AltosGreatCircle(pad_lat, pad_lon, 0, gps.lat, gps.lon, h);
			elevation = from_pad.elevation;
			range = from_pad.range;
			gps_height = gps.alt - pad_alt;
		}
	}

	public void set_tick(int tick) {
		if (tick != AltosRecord.MISSING) {
			if (this.tick != AltosRecord.MISSING) {
				while (tick < this.tick)
					tick += 65536;
				time_change = (tick - this.tick) / 100.0;
			} else
				time_change = 0;
			this.tick = tick;
			update_time();
		}
	}

	public void set_boost_tick(int boost_tick) {
		if (boost_tick != AltosRecord.MISSING)
			this.boost_tick = boost_tick;
	}

	public String state_name() {
		return AltosLib.state_name(state);
	}

	public void set_state(int state) {
		if (state != AltosLib.ao_flight_invalid) {
			this.state = state;
			ascent = (AltosLib.ao_flight_boost <= state &&
				  state <= AltosLib.ao_flight_coast);
			boost = (AltosLib.ao_flight_boost == state);
		}

	}

	public void set_flight(int flight) {

		/* When the flight changes, reset the state */
		if (flight != AltosRecord.MISSING) {
			if (this.flight != AltosRecord.MISSING &&
			    this.flight != flight) {
				init();
			}
			this.flight = flight;
		}
	}

	public void set_serial(int serial) {
		/* When the serial changes, reset the state */
		if (serial != AltosRecord.MISSING) {
			if (this.serial != AltosRecord.MISSING &&
			    this.serial != serial) {
				init();
			}
			this.serial = serial;
		}
	}

	public int rssi() {
		if (rssi == AltosRecord.MISSING)
			return 0;
		return rssi;
	}

	public void set_rssi(int rssi, int status) {
		if (rssi != AltosRecord.MISSING) {
			this.rssi = rssi;
			this.status = status;
		}
	}

	public void set_altitude(double altitude) {
		if (altitude != AltosRecord.MISSING) {
			this.altitude = altitude;
			update_vertical_pos();
			set |= set_position;
		}
	}

	public void set_ground_altitude(double ground_altitude) {
		if (ground_altitude != AltosRecord.MISSING) {
			this.ground_altitude = ground_altitude;
			update_vertical_pos();
		}
	}

	public void set_ground_pressure (double pressure) {
		if (pressure != AltosRecord.MISSING) {
			this.ground_pressure = pressure;
			set_ground_altitude(AltosConvert.pressure_to_altitude(pressure));
			update_vertical_pos();
		}
	}

	public void set_gps(AltosGPS gps, int sequence) {
		if (gps != null) {
			System.out.printf ("gps date: %d-%d-%d time %d:%d:%d\n",
					   gps.year, gps.month, gps.day,
					   gps.hour, gps.minute, gps.second);
			this.gps = gps.clone();
			gps_sequence = sequence;
			update_gps();
			update_vertical_pos();
			set |= set_gps;
		}
	}

	public void set_kalman(double height, double speed, double acceleration) {
		if (height != AltosRecord.MISSING) {
			kalman_height = height;
			kalman_speed = speed;
			kalman_acceleration = acceleration;
			update_vertical_pos();
		}
	}

	public void set_pressure(double pressure) {
		if (pressure != AltosRecord.MISSING) {
			this.pressure = pressure;
			set_altitude(AltosConvert.pressure_to_altitude(pressure));
		}
	}

	public void make_baro() {
		if (baro == null)
			baro = new AltosMs5607();
	}

	public void set_ms5607(int pres, int temp) {
		if (baro != null) {
			baro.set(pres, temp);

			set_pressure(baro.pa);
			set_temperature(baro.cc / 100.0);
		}
	}

	public void make_companion (int nchannels) {
		if (companion == null)
			companion = new AltosRecordCompanion(nchannels);
	}

	public void set_companion(AltosRecordCompanion companion) {
		this.companion = companion;
	}

	public void set_accel_g(double accel_plus_g, double accel_minus_g) {
		if (accel_plus_g != AltosRecord.MISSING) {
			this.accel_plus_g = accel_plus_g;
			this.accel_minus_g = accel_minus_g;
			update_accel();
		}
	}
	public void set_ground_accel(double ground_accel) {
		if (ground_accel != AltosRecord.MISSING) {
			this.ground_accel = ground_accel;
			update_accel();
		}
	}

	public void set_accel(double accel) {
		if (accel != AltosRecord.MISSING) {
			this.accel = accel;
		}
		update_accel();
	}

	public void set_temperature(double temperature) {
		if (temperature != AltosRecord.MISSING) {
			this.temperature = temperature;
			set |= set_data;
		}
	}

	public void set_battery_voltage(double battery_voltage) {
		if (battery_voltage != AltosRecord.MISSING) {
			this.battery_voltage = battery_voltage;
			set |= set_data;
		}
	}

	public void set_pyro_voltage(double pyro_voltage) {
		if (pyro_voltage != AltosRecord.MISSING) {
			this.pyro_voltage = pyro_voltage;
			set |= set_data;
		}
	}

	public void set_apogee_voltage(double apogee_voltage) {
		if (apogee_voltage != AltosRecord.MISSING) {
			this.apogee_voltage = apogee_voltage;
			set |= set_data;
		}
	}

	public void set_main_voltage(double main_voltage) {
		if (main_voltage != AltosRecord.MISSING) {
			this.main_voltage = main_voltage;
			set |= set_data;
		}
	}


	public double time_since_boost() {
		if (tick == AltosRecord.MISSING)
			return 0.0;

		if (boost_tick != AltosRecord.MISSING) {
			return (tick - boost_tick) / 100.0;
		}
		return tick / 100.0;
	}

	public boolean valid() {
		return tick != AltosRecord.MISSING && serial != AltosRecord.MISSING;
	}

	public AltosGPS make_temp_gps() {
		if (temp_gps == null) {
			temp_gps = new AltosGPS(gps);
			temp_gps.cc_gps_sat = null;
		}
		gps_pending = true;
		return temp_gps;
	}

	public void set_temp_gps() {
		set_gps(temp_gps, gps_sequence + 1);
		gps_pending = false;
		temp_gps = null;
	}

	public void init (AltosRecord cur, AltosState prev_state) {

		System.out.printf ("init\n");
		if (cur == null)
			cur = new AltosRecord();

		record = cur;

		/* Discard previous state if it was for a different board */
		if (prev_state != null && prev_state.serial != cur.serial)
			prev_state = null;

		copy(prev_state);

		set_ground_altitude(cur.ground_altitude());
		set_altitude(cur.altitude());

		set_kalman(cur.kalman_height, cur.kalman_speed, cur.kalman_acceleration);

		report_time = System.currentTimeMillis();

		set_temperature(cur.temperature());
		set_apogee_voltage(cur.drogue_voltage());
		set_main_voltage(cur.main_voltage());
		set_battery_voltage(cur.battery_voltage());

		set_pressure(cur.pressure());

		set_tick(cur.tick);
		set_state(cur.state);

		set_accel_g (cur.accel_minus_g, cur.accel_plus_g);
		set_ground_accel(cur.ground_accel);
		set_accel (cur.accel);

		if (cur.gps_sequence != gps_sequence)
			set_gps(cur.gps, cur.gps_sequence);

	}

	public AltosState clone() {
		AltosState s = new AltosState();
		s.copy(this);
		return s;
	}

	public AltosState(AltosRecord cur) {
		init(cur, null);
	}

	public AltosState (AltosRecord cur, AltosState prev) {
		init(cur, prev);
	}

	public AltosState () {
		init();
	}
}

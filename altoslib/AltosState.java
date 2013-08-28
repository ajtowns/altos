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
	public AltosRecord data;

	/* derived data */

	public long  	report_time;

	public double	time;
	public double	time_change;
	public int	tick;

	public int	state;
	public boolean	landed;
	public boolean	ascent;	/* going up? */
	public boolean	boost;	/* under power */

	public double	ground_altitude;
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
	public int	serial;

	public AltosMs5607	baro;

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
		data = new AltosRecord();

		report_time = System.currentTimeMillis();
		time = AltosRecord.MISSING;
		time_change = AltosRecord.MISSING;
		tick = AltosRecord.MISSING;
		state = AltosLib.ao_flight_invalid;
		landed = false;
		boost = false;

		ground_altitude = AltosRecord.MISSING;
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


		accel_speed = AltosRecord.MISSING;
		baro_speed = AltosRecord.MISSING;

		kalman_height = AltosRecord.MISSING;
		kalman_speed = AltosRecord.MISSING;
		kalman_acceleration = AltosRecord.MISSING;

		max_baro_speed = 0;
		max_accel_speed = 0;
		max_height = 0;
		max_acceleration = 0;

		gps = null;
		gps_sequence = 0;

		imu = null;
		mag = null;

		set_npad(0);
		ngps = 0;

		from_pad = null;
		elevation = AltosRecord.MISSING;
		range = AltosRecord.MISSING;
		gps_height = AltosRecord.MISSING;

		pat_lat = AltosRecord.MISSING;
		pad_lon = AltosRecord.MISSING;
		pad_alt = AltosRecord.MISSING;

		speak_tick = AltosRecord.MISSING;
		speak_altitude = AltosRecord.MISSING;

		callsign = null;

		accel_plus_g = AltosRecord.MISSING;
		accel_minus_g = AltosRecord.MISSING;
		log_format = AltosRecord.MISSING;
		serial = AltosRecord.MISSING;

		baro = null;
	}

	void copy(AltosState old) {

		data = null;

		if (old == null) {
			init();
			return;
		}

		report_time = old.report_time;
		time = old.time;
		time_change = old.time_change;
		tick = old.tick;

		state = old.state;
		landed = old.landed;
		ascent = old.ascent;
		boost = old.boost;

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
		accel_speed = old.accel_speed;
		baro_speed = old.baro_speed;

		prev_height = old.height;
		prev_speed = old.speed;
		prev_acceleration = old.acceleration;

		max_height = old.max_height;
		max_acceleration = old.max_acceleration;
		max_accel_speed = old.max_accel_speed;
		max_baro_speed = old.max_baro_speed;

		kalman_height = old.kalman_height;
		kalman_speed = old.kalman_speed;
		kalman_acceleration = old.kalman_acceleration;

		if (old.gps != null)
			gps = old.gps.clone();
		else
			gps = null;
		gps_sequence = old.gps_sequence;

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
		log_format = old.log_format;
		serial = old.serial;

		baro = old.baro;
	}

	double ground_altitude() {
		
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
		if (state == AltosLib.ao_flight_pad) {
			
		}

		if (kalman_height != AltosRecord.MISSING)
			height = kalman_height;
		else if (altitude != AltosRecord.MISSING && ground_altitude != AltosRecord.MISSING)
			height = altitude - ground_altitude;
		else
			height = AltosRecord.MISSING;

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
	}
	
	void update_accel() {
		if (accel == AltosRecord.MISSING)
			return;
		if (ground_Accel == AltosRecord.MISSING)
			return;
		if (accel_plus_g == AltosRecord.MISSING)
			return;
		if (accel_minus_g == AltosRecord.MISSING)
			return;

		double counts_per_g = (accel_minus_g - accel_plus_g) / 2.0;
		double counts_per_mss = counts_per_g / 9.80665;

		acceleration = (ground_accel - accel) / counts_per_mss;
		update_speed();
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

	public void set_state(int state) {
		if (state != AltosLib.ao_flight_invalid) {
			this.state = state;
			ascent = (AltosLib.ao_flight_boost <= state &&
				  state <= AltosLib.ao_flight_coast);
			boost = (AltosLib.ao_flight_boost == state);
		}

	}

	public void set_altitude(double altitude) {
		if (altitude != AltosRecord.MISSING) {
			this.altitude = altitude;
			update_vertical_pos();
		}
	}

	public void set_ground_altitude(double ground_altitude) {
		if (ground_altitude != AltosRecord.MISSING) {
			this.ground_altitude = ground_altitude;
			update_vertical_pos();
		}
	}

	public void set_gps(AltosGPS gps, int sequence) {
		if (gps != null) {
			this.gps = gps.clone();
			gps_sequence = sequence;
			update_vertical_pos();
		}
	}

	public void set_kalman(double height, double speed, double acceleration) {
		if (height != AltosRecord.MISSING) {
			kalman_height = height;
			kalman_speed = speed;
			kalman_acceleration = acceleration;
			baro_speed = accel_speed = speed;
			update_vertical_pos();
		}
	}

	public void set_pressure(double pressure) {
		if (pressure != AltosRecord.MISSING) {
			this.pressure = pressure;
			set_altitude(AltosConvert.pressure_to_altitude(pressure));
		}
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
		if (temperature != AltosRecord.MISSING)
			this.temperature = temperature;
	}

	public void set_battery_voltage(double battery_voltage) {
		if (battery_voltage != AltosRecord.MISSING)
			this.battery_voltage = battery_voltage;
	}

	public void set_pyro_voltage(double pyro_voltage) {
		if (pyro_voltage != AltosRecord.MISSING)
			this.pyro_voltage = pyro_voltage;
	}

	public void set_apogee_voltage(double apogee_voltage) {
		if (apogee_voltage != AltosRecord.MISSING)
			this.apogee_voltage = apogee_voltage;
	}

	public void set_main_voltage(double main_voltage) {
		if (main_voltage != AltosRecord.MISSING)
			this.main_voltage = main_voltage;
	}

	public void init (AltosRecord cur, AltosState prev_state) {

		if (cur == null)
			cur = new AltosRecord();

		data = cur;

		/* Discard previous state if it was for a different board */
		if (prev_state != null && prev_state.serial != cur.serial)
			prev_state = null;

		copy(prev_state);

		set_ground_altitude(data.ground_altitude());
		set_altitude(data.altitude());

		set_kalman(data.kalman_height, data.kalman_speed, data.kalman_acceleration);

		report_time = System.currentTimeMillis();

		set_temperature(data.temperature());
		set_apogee_voltage(data.drogue_voltage());
		set_main_voltage(data.main_voltage());
		set_battery_voltage(data.battery_voltage());

		set_pressure(data.pressure());

		set_tick(data.tick);
		set_state(data.state);

		set_accel_g (data.accel_minus_g, data.accel_plus_g);
		set_ground_accel(data.ground_accel);
		set_accel (data.accel);

		set_gps(data.gps, data.gps_sequence);

		if (prev_state != null) {

			if (data.kalman_speed != AltosRecord.MISSING) {
				baro_speed = accel_speed = data.kalman_speed;
			} else {
				/* compute barometric speed */

				double height_change = height - prev_state.height;

				double prev_baro_speed = prev_state.baro_speed;
				if (prev_baro_speed == AltosRecord.MISSING)
					prev_baro_speed = 0;

				if (time_change > 0)
					baro_speed = (prev_baro_speed * 3 + (height_change / time_change)) / 4.0;
				else
					baro_speed = prev_state.baro_speed;

				double prev_accel_speed = prev_state.accel_speed;

				if (prev_accel_speed == AltosRecord.MISSING)
					prev_accel_speed = 0;

				if (acceleration == AltosRecord.MISSING) {
					/* Fill in mising acceleration value */
					accel_speed = baro_speed;

					if (time_change > 0 && accel_speed != AltosRecord.MISSING)
						acceleration = (accel_speed - prev_accel_speed) / time_change;
					else
						acceleration = prev_state.acceleration;
				} else {
					/* compute accelerometer speed */
					accel_speed = prev_accel_speed + acceleration * time_change;
				}
			}
		} else {
			npad = 0;
			ngps = 0;
			gps = null;
			gps_sequence = 0;
			baro_speed = AltosRecord.MISSING;
			accel_speed = AltosRecord.MISSING;
			pad_alt = AltosRecord.MISSING;
			max_baro_speed = 0;
			max_accel_speed = 0;
			max_height = 0;
			max_acceleration = 0;
			time_change = 0;
			baro = new AltosMs5607();
			callsign = "";
			accel_plus_g = AltosRecord.MISSING;
			accel_minus_g = AltosRecord.MISSING;
			log_format = AltosRecord.MISSING;
			serial = AltosRecord.MISSING;
		}

		time = tick / 100.0;

		if (data.gps != null && data.gps_sequence != gps_sequence && (state < AltosLib.ao_flight_boost)) {

			/* Track consecutive 'good' gps reports, waiting for 10 of them */
			if (data.gps != null && data.gps.locked && data.gps.nsat >= 4)
				set_npad(npad+1);
			else
				set_npad(0);

			/* Average GPS data while on the pad */
			if (data.gps != null && data.gps.locked && data.gps.nsat >= 4) {
				if (ngps > 1 && state == AltosLib.ao_flight_pad) {
					/* filter pad position */
					pad_lat = (pad_lat * 31.0 + data.gps.lat) / 32.0;
					pad_lon = (pad_lon * 31.0 + data.gps.lon) / 32.0;
					pad_alt = (pad_alt * 31.0 + data.gps.alt) / 32.0;
				} else {
					pad_lat = data.gps.lat;
					pad_lon = data.gps.lon;
					pad_alt = data.gps.alt;
				}
				ngps++;
			}
		} else {
			if (ngps == 0 && ground_altitude != AltosRecord.MISSING)
				pad_alt = ground_altitude;
		}

		gps_sequence = data.gps_sequence;

		/* Only look at accelerometer data under boost */
		if (boost && acceleration != AltosRecord.MISSING && (max_acceleration == AltosRecord.MISSING || acceleration > max_acceleration))
			max_acceleration = acceleration;
		if (boost && accel_speed != AltosRecord.MISSING && accel_speed > max_accel_speed)
			max_accel_speed = accel_speed;
		if (boost && baro_speed != AltosRecord.MISSING && baro_speed > max_baro_speed)
			max_baro_speed = baro_speed;

		if (height != AltosRecord.MISSING && height > max_height)
			max_height = height;
		elevation = 0;
		range = -1;
		gps_height = 0;
		if (data.gps != null) {
			gps = data.gps;
			if (ngps > 0 && gps.locked) {
				double h = height;

				if (h == AltosRecord.MISSING) h = 0;
				from_pad = new AltosGreatCircle(pad_lat, pad_lon, 0, gps.lat, gps.lon, h);
				elevation = from_pad.elevation;
				range = from_pad.range;
				gps_height = gps.alt - pad_alt;
			}
		}
	}

	public AltosState clone() {
		AltosState s = new AltosState(data, this);
		return s;
	}

	public AltosState(AltosRecord cur) {
		init(cur, null);
	}

	public AltosState (AltosRecord cur, AltosState prev) {
		init(cur, prev);
	}
}

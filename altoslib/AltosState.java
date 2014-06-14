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

package org.altusmetrum.altoslib_4;

public class AltosState implements Cloneable {

	public static final int set_position = 1;
	public static final int set_gps = 2;
	public static final int set_data = 4;

	public int set;

	static final double ascent_filter_len = 0.5;
	static final double descent_filter_len = 0.5;

	/* derived data */

	public long	received_time;

	public double	time;
	public double	prev_time;
	public double	time_change;
	public int	tick;
	private int	prev_tick;
	public int	boost_tick;

	class AltosValue {
		private double	value;
		private double	prev_value;
		private double	max_value;
		private double	set_time;
		private double	prev_set_time;

		boolean can_max() { return true; }

		void set(double new_value, double time) {
			if (new_value != AltosLib.MISSING) {
				value = new_value;
				if (can_max() && (max_value == AltosLib.MISSING || value > max_value))
					max_value = value;
				set_time = time;
			}
		}

		void set_filtered(double new_value, double time) {
			if (prev_value != AltosLib.MISSING)
				new_value = (prev_value * 15.0 + new_value) / 16.0;
			set(new_value, time);
		}

		double value() {
			return value;
		}

		double max() {
			return max_value;
		}

		double prev() {
			return prev_value;
		}

		double change() {
			if (value != AltosLib.MISSING && prev_value != AltosLib.MISSING)
				return value - prev_value;
			return AltosLib.MISSING;
		}

		double rate() {
			double c = change();
			double t = set_time - prev_set_time;

			if (c != AltosLib.MISSING && t != 0)
				return c / t;
			return AltosLib.MISSING;
		}

		double integrate() {
			if (value == AltosLib.MISSING)
				return AltosLib.MISSING;
			if (prev_value == AltosLib.MISSING)
				return AltosLib.MISSING;

			return (value + prev_value) / 2 * (set_time - prev_set_time);
		}

		double time() {
			return set_time;
		}

		void set_derivative(AltosValue in) {
			double	n = in.rate();

			if (n == AltosLib.MISSING)
				return;

			double	p = prev_value;
			double	pt = prev_set_time;

			if (p == AltosLib.MISSING) {
				p = 0;
				pt = in.time() - 0.01;
			}

			/* Clip changes to reduce noise */
			double	ddt = in.time() - pt;
			double	ddv = (n - p) / ddt;

			final double max = 100000;

			/* 100gs */
			if (Math.abs(ddv) > max) {
				if (n > p)
					n = p + ddt * max;
				else
					n = p - ddt * max;
			}

			double filter_len;

			if (ascent)
				filter_len = ascent_filter_len;
			else
				filter_len = descent_filter_len;

			double f = 1/Math.exp(ddt/ filter_len);
			n = p * f + n * (1-f);

			set(n, in.time());
		}

		void set_integral(AltosValue in) {
			double	change = in.integrate();

			if (change != AltosLib.MISSING) {
				double	prev = prev_value;
				if (prev == AltosLib.MISSING)
					prev = 0;
				set(prev + change, in.time());
			}
		}

		void copy(AltosValue old) {
			value = old.value;
			set_time = old.set_time;
			prev_value = old.value;
			prev_set_time = old.set_time;
			max_value = old.max_value;
		}

		void finish_update() {
			prev_value = value;
			prev_set_time = set_time;
		}

		AltosValue() {
			value = AltosLib.MISSING;
			prev_value = AltosLib.MISSING;
			max_value = AltosLib.MISSING;
		}
	}

	class AltosCValue {
		AltosValue	measured;
		AltosValue	computed;

		double value() {
			double v = measured.value();
			if (v != AltosLib.MISSING)
				return v;
			return computed.value();
		}

		boolean is_measured() {
			return measured.value() != AltosLib.MISSING;
		}

		double max() {
			double m = measured.max();

			if (m != AltosLib.MISSING)
				return m;
			return computed.max();
		}

		double prev_value() {
			if (measured.value != AltosLib.MISSING && measured.prev_value != AltosLib.MISSING)
				return measured.prev_value;
			return computed.prev_value;
		}

		AltosValue altos_value() {
			if (measured.value() != AltosLib.MISSING)
				return measured;
			return computed;
		}

		double change() {
			double c = measured.change();
			if (c == AltosLib.MISSING)
				c = computed.change();
			return c;
		}

		double rate() {
			double r = measured.rate();
			if (r == AltosLib.MISSING)
				r = computed.rate();
			return r;
		}

		void set_measured(double new_value, double time) {
			measured.set(new_value, time);
		}

		void set_computed(double new_value, double time) {
			computed.set(new_value, time);
		}

		void set_derivative(AltosValue in) {
			computed.set_derivative(in);
		}

		void set_derivative(AltosCValue in) {
			set_derivative(in.altos_value());
		}

		void set_integral(AltosValue in) {
			computed.set_integral(in);
		}

		void set_integral(AltosCValue in) {
			set_integral(in.altos_value());
		}

		void copy(AltosCValue old) {
			measured.copy(old.measured);
			computed.copy(old.computed);
		}

		void finish_update() {
			measured.finish_update();
			computed.finish_update();
		}

		AltosCValue() {
			measured = new AltosValue();
			computed = new AltosValue();
		}
	}

	public int	state;
	public int	flight;
	public int	serial;
	public int	receiver_serial;
	public boolean	landed;
	public boolean	ascent;	/* going up? */
	public boolean	boost;	/* under power */
	public int	rssi;
	public int	status;
	public int	device_type;
	public int	config_major;
	public int	config_minor;
	public int	apogee_delay;
	public int	main_deploy;
	public int	flight_log_max;

	private double pressure_to_altitude(double p) {
		if (p == AltosLib.MISSING)
			return AltosLib.MISSING;
		return AltosConvert.pressure_to_altitude(p);
	}

	private AltosCValue ground_altitude;

	public double ground_altitude() {
		return ground_altitude.value();
	}

	public void set_ground_altitude(double a) {
		ground_altitude.set_measured(a, time);
	}

	class AltosGpsGroundAltitude extends AltosValue {
		void set(double a, double t) {
			super.set(a, t);
			pad_alt = value();
			gps_altitude.set_gps_height();
		}

		void set_filtered(double a, double t) {
			super.set_filtered(a, t);
			pad_alt = value();
			gps_altitude.set_gps_height();
		}
	}

	private AltosGpsGroundAltitude gps_ground_altitude;

	public double gps_ground_altitude() {
		return gps_ground_altitude.value();
	}

	public void set_gps_ground_altitude(double a) {
		gps_ground_altitude.set(a, time);
	}

	class AltosGroundPressure extends AltosCValue {
		void set_filtered(double p, double time) {
			computed.set_filtered(p, time);
			if (!is_measured())
				ground_altitude.set_computed(pressure_to_altitude(computed.value()), time);
		}

		void set_measured(double p, double time) {
			super.set_measured(p, time);
			ground_altitude.set_computed(pressure_to_altitude(p), time);
		}
	}

	private AltosGroundPressure ground_pressure;

	public double ground_pressure() {
		return ground_pressure.value();
	}

	public void set_ground_pressure (double pressure) {
		ground_pressure.set_measured(pressure, time);
	}

	class AltosAltitude extends AltosCValue {

		private void set_speed(AltosValue v) {
			if (!acceleration.is_measured() || !ascent)
				speed.set_derivative(this);
		}

		void set_computed(double a, double time) {
			super.set_computed(a,time);
			set_speed(computed);
			set |= set_position;
		}

		void set_measured(double a, double time) {
			super.set_measured(a,time);
			set_speed(measured);
			set |= set_position;
		}
	}

	private AltosAltitude	altitude;

	class AltosGpsAltitude extends AltosValue {

		private void set_gps_height() {
			double	a = value();
			double	g = gps_ground_altitude.value();

			if (a != AltosLib.MISSING && g != AltosLib.MISSING)
				gps_height = a - g;
			else
				gps_height = AltosLib.MISSING;
		}

		void set(double a, double t) {
			super.set(a, t);
			set_gps_height();
		}
	}

	private AltosGpsAltitude	gps_altitude;

	private AltosValue		gps_ground_speed;
	private AltosValue		gps_ascent_rate;
	private AltosValue		gps_course;
	private AltosValue		gps_speed;

	public double altitude() {
		double a = altitude.value();
		if (a != AltosLib.MISSING)
			return a;
		return gps_altitude.value();
	}

	public double max_altitude() {
		double a = altitude.max();
		if (a != AltosLib.MISSING)
			return a;
		return gps_altitude.max();
	}

	public void set_altitude(double new_altitude) {
		altitude.set_measured(new_altitude, time);
	}

	public double gps_altitude() {
		return gps_altitude.value();
	}

	public double max_gps_altitude() {
		return gps_altitude.max();
	}

	public void set_gps_altitude(double new_gps_altitude) {
		gps_altitude.set(new_gps_altitude, time);
	}

	public double gps_ground_speed() {
		return gps_ground_speed.value();
	}

	public double max_gps_ground_speed() {
		return gps_ground_speed.max();
	}

	public double gps_ascent_rate() {
		return gps_ascent_rate.value();
	}

	public double max_gps_ascent_rate() {
		return gps_ascent_rate.max();
	}

	public double gps_course() {
		return gps_course.value();
	}

	public double gps_speed() {
		return gps_speed.value();
	}

	public double max_gps_speed() {
		return gps_speed.max();
	}

	class AltosPressure extends AltosValue {
		void set(double p, double time) {
			super.set(p, time);
			if (state == AltosLib.ao_flight_pad)
				ground_pressure.set_filtered(p, time);
			double a = pressure_to_altitude(p);
			altitude.set_computed(a, time);
		}
	}

	private AltosPressure	pressure;

	public double pressure() {
		return pressure.value();
	}

	public void set_pressure(double p) {
		pressure.set(p, time);
	}

	public double height() {
		double k = kalman_height.value();
		if (k != AltosLib.MISSING)
			return k;

		double a = altitude();
		double g = ground_altitude();
		if (a != AltosLib.MISSING && g != AltosLib.MISSING)
			return a - g;
		return gps_height();
	}

	public double max_height() {
		double	k = kalman_height.max();
		if (k != AltosLib.MISSING)
			return k;

		double a = altitude.max();
		double g = ground_altitude();
		if (a != AltosLib.MISSING && g != AltosLib.MISSING)
			return a - g;
		return max_gps_height();
	}

	public double gps_height() {
		double a = gps_altitude();
		double g = gps_ground_altitude();

		if (a != AltosLib.MISSING && g != AltosLib.MISSING)
			return a - g;
		return AltosLib.MISSING;
	}

	public double max_gps_height() {
		double a = gps_altitude.max();
		double g = gps_ground_altitude();

		if (a != AltosLib.MISSING && g != AltosLib.MISSING)
			return a - g;
		return AltosLib.MISSING;
	}

	class AltosSpeed extends AltosCValue {

		boolean can_max() {
			return state < AltosLib.ao_flight_fast || state == AltosLib.ao_flight_stateless;
		}

		void set_accel() {
			acceleration.set_derivative(this);
		}

		void set_derivative(AltosCValue in) {
			super.set_derivative(in);
			set_accel();
		}

		void set_computed(double new_value, double time) {
			super.set_computed(new_value, time);
			set_accel();
		}

		void set_measured(double new_value, double time) {
			super.set_measured(new_value, time);
			set_accel();
		}
	}

	private AltosSpeed speed;

	public double speed() {
		double v = kalman_speed.value();
		if (v != AltosLib.MISSING)
			return v;
		v = speed.value();
		if (v != AltosLib.MISSING)
			return v;
		v = gps_speed();
		if (v != AltosLib.MISSING)
			return v;
		return AltosLib.MISSING;
	}

	public double max_speed() {
		double v = kalman_speed.max();
		if (v != AltosLib.MISSING)
			return v;
		v = speed.max();
		if (v != AltosLib.MISSING)
			return v;
		v = max_gps_speed();
		if (v != AltosLib.MISSING)
			return v;
		return AltosLib.MISSING;
	}

	class AltosAccel extends AltosCValue {

		boolean can_max() {
			return state < AltosLib.ao_flight_fast || state == AltosLib.ao_flight_stateless;
		}

		void set_measured(double a, double time) {
			super.set_measured(a, time);
			if (ascent)
				speed.set_integral(this.measured);
		}
	}

	AltosAccel acceleration;

	public double acceleration() {
		return acceleration.value();
	}

	public double max_acceleration() {
		return acceleration.max();
	}

	public AltosValue	orient;

	public void set_orient(double new_orient) {
		orient.set(new_orient, time);
	}

	public double orient() {
		return orient.value();
	}

	public double max_orient() {
		return orient.max();
	}

	public AltosValue	kalman_height, kalman_speed, kalman_acceleration;

	public void set_kalman(double height, double speed, double acceleration) {
		kalman_height.set(height, time);
		kalman_speed.set(speed, time);
		kalman_acceleration.set(acceleration, time);
	}

	public double	battery_voltage;
	public double	pyro_voltage;
	public double	temperature;
	public double	apogee_voltage;
	public double	main_voltage;

	public double	ignitor_voltage[];

	public AltosGPS	gps;
	public AltosGPS	temp_gps;
	public int temp_gps_sat_tick;
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
	public String	firmware_version;

	public double	accel_plus_g;
	public double	accel_minus_g;
	public double	accel;
	public double	ground_accel;
	public double	ground_accel_avg;

	public int	log_format;
	public String	product;

	public AltosMs5607	baro;

	public AltosCompanion	companion;

	public int	pyro_fired;

	public void set_npad(int npad) {
		this.npad = npad;
		gps_waiting = MIN_PAD_SAMPLES - npad;
		if (this.gps_waiting < 0)
			gps_waiting = 0;
		gps_ready = gps_waiting == 0;
	}

	public void init() {
		set = 0;

		received_time = System.currentTimeMillis();
		time = AltosLib.MISSING;
		time_change = AltosLib.MISSING;
		prev_time = AltosLib.MISSING;
		tick = AltosLib.MISSING;
		prev_tick = AltosLib.MISSING;
		boost_tick = AltosLib.MISSING;
		state = AltosLib.ao_flight_invalid;
		flight = AltosLib.MISSING;
		landed = false;
		boost = false;
		rssi = AltosLib.MISSING;
		status = 0;
		device_type = AltosLib.MISSING;
		config_major = AltosLib.MISSING;
		config_minor = AltosLib.MISSING;
		apogee_delay = AltosLib.MISSING;
		main_deploy = AltosLib.MISSING;
		flight_log_max = AltosLib.MISSING;

		ground_altitude = new AltosCValue();
		ground_pressure = new AltosGroundPressure();
		altitude = new AltosAltitude();
		pressure = new AltosPressure();
		speed = new AltosSpeed();
		acceleration = new AltosAccel();
		orient = new AltosValue();

		temperature = AltosLib.MISSING;
		battery_voltage = AltosLib.MISSING;
		pyro_voltage = AltosLib.MISSING;
		apogee_voltage = AltosLib.MISSING;
		main_voltage = AltosLib.MISSING;
		ignitor_voltage = null;

		kalman_height = new AltosValue();
		kalman_speed = new AltosValue();
		kalman_acceleration = new AltosValue();

		gps = null;
		temp_gps = null;
		temp_gps_sat_tick = 0;
		gps_sequence = 0;
		gps_pending = false;

		imu = null;
		mag = null;

		set_npad(0);
		ngps = 0;

		from_pad = null;
		elevation = AltosLib.MISSING;
		range = AltosLib.MISSING;
		gps_height = AltosLib.MISSING;

		pad_lat = AltosLib.MISSING;
		pad_lon = AltosLib.MISSING;
		pad_alt = AltosLib.MISSING;

		gps_altitude = new AltosGpsAltitude();
		gps_ground_altitude = new AltosGpsGroundAltitude();
		gps_ground_speed = new AltosValue();
		gps_speed = new AltosValue();
		gps_ascent_rate = new AltosValue();
		gps_course = new AltosValue();

		speak_tick = AltosLib.MISSING;
		speak_altitude = AltosLib.MISSING;

		callsign = null;
		firmware_version = null;

		accel_plus_g = AltosLib.MISSING;
		accel_minus_g = AltosLib.MISSING;
		accel = AltosLib.MISSING;

		ground_accel = AltosLib.MISSING;
		ground_accel_avg = AltosLib.MISSING;

		log_format = AltosLib.MISSING;
		product = null;
		serial = AltosLib.MISSING;
		receiver_serial = AltosLib.MISSING;

		baro = null;
		companion = null;

		pyro_fired = 0;
	}

	void finish_update() {
		prev_tick = tick;

		ground_altitude.finish_update();
		altitude.finish_update();
		pressure.finish_update();
		speed.finish_update();
		acceleration.finish_update();
		orient.finish_update();

		kalman_height.finish_update();
		kalman_speed.finish_update();
		kalman_acceleration.finish_update();
	}

	void copy(AltosState old) {

		if (old == null) {
			init();
			return;
		}

		received_time = old.received_time;
		time = old.time;
		time_change = old.time_change;
		prev_time = old.time;

		tick = old.tick;
		prev_tick = old.tick;
		boost_tick = old.boost_tick;

		state = old.state;
		flight = old.flight;
		landed = old.landed;
		ascent = old.ascent;
		boost = old.boost;
		rssi = old.rssi;
		status = old.status;
		device_type = old.device_type;
		config_major = old.config_major;
		config_minor = old.config_minor;
		apogee_delay = old.apogee_delay;
		main_deploy = old.main_deploy;
		flight_log_max = old.flight_log_max;

		set = 0;

		ground_pressure.copy(old.ground_pressure);
		ground_altitude.copy(old.ground_altitude);
		altitude.copy(old.altitude);
		pressure.copy(old.pressure);
		speed.copy(old.speed);
		acceleration.copy(old.acceleration);
		orient.copy(old.orient);

		battery_voltage = old.battery_voltage;
		pyro_voltage = old.pyro_voltage;
		temperature = old.temperature;
		apogee_voltage = old.apogee_voltage;
		main_voltage = old.main_voltage;
		ignitor_voltage = old.ignitor_voltage;

		kalman_height.copy(old.kalman_height);
		kalman_speed.copy(old.kalman_speed);
		kalman_acceleration.copy(old.kalman_acceleration);

		if (old.gps != null)
			gps = old.gps.clone();
		else
			gps = null;
		if (old.temp_gps != null)
			temp_gps = old.temp_gps.clone();
		else
			temp_gps = null;
		temp_gps_sat_tick = old.temp_gps_sat_tick;
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

		gps_altitude.copy(old.gps_altitude);
		gps_ground_altitude.copy(old.gps_ground_altitude);
		gps_ground_speed.copy(old.gps_ground_speed);
		gps_ascent_rate.copy(old.gps_ascent_rate);
		gps_course.copy(old.gps_course);
		gps_speed.copy(old.gps_speed);

		pad_lat = old.pad_lat;
		pad_lon = old.pad_lon;
		pad_alt = old.pad_alt;

		speak_tick = old.speak_tick;
		speak_altitude = old.speak_altitude;

		callsign = old.callsign;
		firmware_version = old.firmware_version;

		accel_plus_g = old.accel_plus_g;
		accel_minus_g = old.accel_minus_g;
		accel = old.accel;
		ground_accel = old.ground_accel;
		ground_accel_avg = old.ground_accel_avg;

		log_format = old.log_format;
		product = old.product;
		serial = old.serial;
		receiver_serial = old.receiver_serial;

		baro = old.baro;
		companion = old.companion;

		pyro_fired = old.pyro_fired;
	}

	void update_time() {
	}

	void update_gps() {
		elevation = AltosLib.MISSING;
		range = AltosLib.MISSING;

		if (gps == null)
			return;

		if (gps.locked && gps.nsat >= 4) {
			/* Track consecutive 'good' gps reports, waiting for 10 of them */
			if (state == AltosLib.ao_flight_pad || state == AltosLib.ao_flight_stateless) {
				set_npad(npad+1);
				if (pad_lat != AltosLib.MISSING && (npad < 10 || state == AltosLib.ao_flight_pad)) {
					pad_lat = (pad_lat * 31 + gps.lat) / 32;
					pad_lon = (pad_lon * 31 + gps.lon) / 32;
					gps_ground_altitude.set_filtered(gps.alt, time);
				}
			}
			if (pad_lat == AltosLib.MISSING) {
				pad_lat = gps.lat;
				pad_lon = gps.lon;
				gps_ground_altitude.set(gps.alt, time);
			}
			gps_altitude.set(gps.alt, time);
			if (gps.climb_rate != AltosLib.MISSING)
				gps_ascent_rate.set(gps.climb_rate, time);
			if (gps.ground_speed != AltosLib.MISSING)
				gps_ground_speed.set(gps.ground_speed, time);
			if (gps.climb_rate != AltosLib.MISSING && gps.ground_speed != AltosLib.MISSING)
				gps_speed.set(Math.sqrt(gps.ground_speed * gps.ground_speed +
							gps.climb_rate * gps.climb_rate), time);
			if (gps.course != AltosLib.MISSING)
				gps_course.set(gps.course, time);
		}
		if (gps.lat != 0 && gps.lon != 0 &&
		    pad_lat != AltosLib.MISSING &&
		    pad_lon != AltosLib.MISSING)
		{
			double h = height();

			if (h == AltosLib.MISSING)
				h = 0;
			from_pad = new AltosGreatCircle(pad_lat, pad_lon, 0, gps.lat, gps.lon, h);
			elevation = from_pad.elevation;
			range = from_pad.range;
		}
	}

	public void set_tick(int new_tick) {
		if (new_tick != AltosLib.MISSING) {
			if (prev_tick != AltosLib.MISSING) {
				while (new_tick < prev_tick - 1000) {
					new_tick += 65536;
				}
			}
			tick = new_tick;
			time = tick / 100.0;
			time_change = time - prev_time;
		}
	}

	public void set_boost_tick(int boost_tick) {
		if (boost_tick != AltosLib.MISSING)
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

	public void set_device_type(int device_type) {
		this.device_type = device_type;
		switch (device_type) {
		case AltosLib.product_telegps:
			this.state = AltosLib.ao_flight_stateless;
			break;
		}
	}

	public void set_log_format(int log_format) {
		this.log_format = log_format;
		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_TELEGPS:
			this.state = AltosLib.ao_flight_stateless;
			break;
		}
	}

	public void set_flight_params(int apogee_delay, int main_deploy) {
		this.apogee_delay = apogee_delay;
		this.main_deploy = main_deploy;
	}

	public void set_config(int major, int minor, int flight_log_max) {
		config_major = major;
		config_minor = minor;
		this.flight_log_max = flight_log_max;
	}

	public void set_callsign(String callsign) {
		this.callsign = callsign;
	}

	public void set_firmware_version(String version) {
		firmware_version = version;
	}

	public void set_flight(int flight) {

		/* When the flight changes, reset the state */
		if (flight != AltosLib.MISSING && flight != 0) {
			if (this.flight != AltosLib.MISSING &&
			    this.flight != flight) {
				int bt = boost_tick;
				init();
				boost_tick = bt;
			}
			this.flight = flight;
		}
	}

	public void set_serial(int serial) {
		/* When the serial changes, reset the state */
		if (serial != AltosLib.MISSING) {
			if (this.serial != AltosLib.MISSING &&
			    this.serial != serial) {
				int bt = boost_tick;
				init();
				boost_tick = bt;
			}
			this.serial = serial;
		}
	}

	public void set_receiver_serial(int serial) {
		if (serial != AltosLib.MISSING)
			receiver_serial = serial;
	}

	public int rssi() {
		if (rssi == AltosLib.MISSING)
			return 0;
		return rssi;
	}

	public void set_rssi(int rssi, int status) {
		if (rssi != AltosLib.MISSING) {
			this.rssi = rssi;
			this.status = status;
		}
	}

	public void set_received_time(long ms) {
		received_time = ms;
	}

	public void set_gps(AltosGPS gps, int sequence) {
		if (gps != null) {
			this.gps = gps.clone();
			gps_sequence = sequence;
			update_gps();
			set |= set_gps;
		}
	}

	public void set_imu(AltosIMU imu) {
		if (imu != null)
			imu = imu.clone();
		this.imu = imu;
	}

	public void set_mag(AltosMag mag) {
		this.mag = mag.clone();
	}

	public AltosMs5607 make_baro() {
		if (baro == null)
			baro = new AltosMs5607();
		return baro;
	}

	public void set_ms5607(AltosMs5607 ms5607) {
		baro = ms5607;

		if (baro != null) {
			set_pressure(baro.pa);
			set_temperature(baro.cc / 100.0);
		}
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
			companion = new AltosCompanion(nchannels);
	}

	public void set_companion(AltosCompanion companion) {
		this.companion = companion;
	}

	void update_accel() {
		if (accel == AltosLib.MISSING)
			return;
		if (accel_plus_g == AltosLib.MISSING)
			return;
		if (accel_minus_g == AltosLib.MISSING)
			return;

		double counts_per_g = (accel_minus_g - accel_plus_g) / 2.0;
		double counts_per_mss = counts_per_g / 9.80665;
		acceleration.set_measured((accel_plus_g - accel) / counts_per_mss, time);
	}

	public void set_accel_g(double accel_plus_g, double accel_minus_g) {
		if (accel_plus_g != AltosLib.MISSING) {
			this.accel_plus_g = accel_plus_g;
			this.accel_minus_g = accel_minus_g;
			update_accel();
		}
	}

	public void set_ground_accel(double ground_accel) {
		if (ground_accel != AltosLib.MISSING)
			this.ground_accel = ground_accel;
	}

	public void set_accel(double accel) {
		if (accel != AltosLib.MISSING) {
			this.accel = accel;
			if (state == AltosLib.ao_flight_pad) {
				if (ground_accel_avg == AltosLib.MISSING)
					ground_accel_avg = accel;
				else
					ground_accel_avg = (ground_accel_avg * 7 + accel) / 8;
			}
		}
		update_accel();
	}

	public void set_temperature(double temperature) {
		if (temperature != AltosLib.MISSING) {
			this.temperature = temperature;
			set |= set_data;
		}
	}

	public void set_battery_voltage(double battery_voltage) {
		if (battery_voltage != AltosLib.MISSING) {
			this.battery_voltage = battery_voltage;
			set |= set_data;
		}
	}

	public void set_pyro_voltage(double pyro_voltage) {
		if (pyro_voltage != AltosLib.MISSING) {
			this.pyro_voltage = pyro_voltage;
			set |= set_data;
		}
	}

	public void set_apogee_voltage(double apogee_voltage) {
		if (apogee_voltage != AltosLib.MISSING) {
			this.apogee_voltage = apogee_voltage;
			set |= set_data;
		}
	}

	public void set_main_voltage(double main_voltage) {
		if (main_voltage != AltosLib.MISSING) {
			this.main_voltage = main_voltage;
			set |= set_data;
		}
	}

	public void set_ignitor_voltage(double[] voltage) {
		this.ignitor_voltage = voltage;
	}

	public void set_pyro_fired(int fired) {
		this.pyro_fired = fired;
	}

	public double time_since_boost() {
		if (tick == AltosLib.MISSING)
			return 0.0;

		if (boost_tick == AltosLib.MISSING)
			return tick / 100.0;
		return (tick - boost_tick) / 100.0;
	}

	public boolean valid() {
		return tick != AltosLib.MISSING && serial != AltosLib.MISSING;
	}

	public AltosGPS make_temp_gps(boolean sats) {
		if (temp_gps == null) {
			temp_gps = new AltosGPS(gps);
		}
		gps_pending = true;
		if (sats) {
			if (tick != temp_gps_sat_tick)
				temp_gps.cc_gps_sat = null;
			temp_gps_sat_tick = tick;
		}
		return temp_gps;
	}

	public void set_temp_gps() {
		set_gps(temp_gps, gps_sequence + 1);
		gps_pending = false;
		temp_gps = null;
	}

	public AltosState clone() {
		AltosState s = new AltosState();
		s.copy(this);
		return s;
	}

	public AltosState () {
		init();
	}
}

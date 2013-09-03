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

	static final double ascent_filter_len = 0.1;
	static final double descent_filter_len = 2.0;

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
		private double	max_rate = 1000.0;

		void set(double new_value, double time) {
			if (new_value != AltosRecord.MISSING) {
				value = new_value;
				if (max_value == AltosRecord.MISSING || value > max_value) {
					max_value = value;
				}
				set_time = time;
			}
		}

		void set_filtered(double new_value, double time) {
			if (prev_value != AltosRecord.MISSING)
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
			if (value != AltosRecord.MISSING && prev_value != AltosRecord.MISSING)
				return value - prev_value;
			return AltosRecord.MISSING;
		}

		double rate() {
			double c = change();
			double t = set_time - prev_set_time;

			if (c != AltosRecord.MISSING && t != 0)
				return c / t;
			return AltosRecord.MISSING;
		}

		double integrate() {
			if (value == AltosRecord.MISSING)
				return AltosRecord.MISSING;
			if (prev_value == AltosRecord.MISSING)
				return AltosRecord.MISSING;

			return (value + prev_value) / 2 * (set_time - prev_set_time);
		}

		double time() {
			return set_time;
		}

		void set_derivative(AltosValue in) {
			double	n = in.rate();
			
			if (n == AltosRecord.MISSING)
				return;

			double	p = prev_value;
			double	pt = prev_set_time;

			if (p == AltosRecord.MISSING) {
				p = 0;
				pt = in.time() - 0.01;
			}

			/* Clip changes to reduce noise */
			double	ddt = in.time() - pt;
			double	ddv = (n - p) / ddt;
				
			/* 100gs */
			if (Math.abs(ddv) > 1000) {
				if (n > p)
					n = p + ddt * 1000;
				else
					n = p - ddt * 1000;
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

			if (change != AltosRecord.MISSING)
				set(prev_value + change, in.time());
		}

		void copy(AltosValue old) {
			value = old.value;
			set_time = old.set_time;
			prev_value = old.value;
			prev_set_time = old.set_time;
			max_value = old.max_value;
		}

		AltosValue() {
			value = AltosRecord.MISSING;
			prev_value = AltosRecord.MISSING;
			max_value = AltosRecord.MISSING;
		}
	}

	class AltosCValue {
		AltosValue	measured;
		AltosValue	computed;

		double value() {
			double v = measured.value();
			if (v != AltosRecord.MISSING)
				return v;
			return computed.value();
		}

		boolean is_measured() {
			return measured.value() != AltosRecord.MISSING;
		}

		double max() {
			double m = measured.max();

			if (m != AltosRecord.MISSING)
				return m;
			return computed.max();
		}

		double prev_value() {
			if (measured.value != AltosRecord.MISSING && measured.prev_value != AltosRecord.MISSING)
				return measured.prev_value;
			return computed.prev_value;
		}

		AltosValue altos_value() {
			if (measured.value() != AltosRecord.MISSING)
				return measured;
			return computed;
		}

		double change() {
			double c = measured.change();
			if (c == AltosRecord.MISSING)
				c = computed.change();
			return c;
		}

		double rate() {
			double r = measured.rate();
			if (r == AltosRecord.MISSING)
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

		AltosCValue() {
			measured = new AltosValue();
			computed = new AltosValue();
		}
	}

	public int	state;
	public int	flight;
	public int	serial;
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
		if (p == AltosRecord.MISSING)
			return AltosRecord.MISSING;
		return AltosConvert.pressure_to_altitude(p);
	}

	private AltosCValue ground_altitude;

	public double ground_altitude() {
		return ground_altitude.value();
	}

	public void set_ground_altitude(double a) {
		ground_altitude.set_measured(a, time);
	}

	class AltosGroundPressure extends AltosCValue {
		void set_filtered(double p, double time) {
			computed.set_filtered(p, time);
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

	public double altitude() {
		double a = altitude.value();
		if (a != AltosRecord.MISSING)
			return a;
		if (gps != null)
			return gps.alt;
		return AltosRecord.MISSING;
	}

	public double max_altitude() {
		double a = altitude.max();
		if (a != AltosRecord.MISSING)
			return a;
		return AltosRecord.MISSING;
	}

	public void set_altitude(double new_altitude) {
		altitude.set_measured(new_altitude, time);
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
		if (k != AltosRecord.MISSING)
			return k;

		double a = altitude();
		double g = ground_altitude();
		if (a != AltosRecord.MISSING && g != AltosRecord.MISSING)
			return a - g;
		return AltosRecord.MISSING;
	}

	public double max_height() {
		double	k = kalman_height.max();
		if (k != AltosRecord.MISSING)
			return k;

		double a = altitude.max();
		double g = ground_altitude();
		if (a != AltosRecord.MISSING && g != AltosRecord.MISSING)
			return a - g;
		return AltosRecord.MISSING;
	}

	class AltosSpeed extends AltosCValue {
		
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
		return speed.value();
	}

	public double max_speed() {
		return speed.max();
	}

	class AltosAccel extends AltosCValue {
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
	public boolean	temp_gps_clear_sats_pending;
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

	public AltosMs5607	baro;

	public AltosRecordCompanion	companion;

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

		received_time = System.currentTimeMillis();
		time = AltosRecord.MISSING;
		time_change = AltosRecord.MISSING;
		prev_time = AltosRecord.MISSING;
		tick = AltosRecord.MISSING;
		prev_tick = AltosRecord.MISSING;
		boost_tick = AltosRecord.MISSING;
		state = AltosLib.ao_flight_invalid;
		flight = AltosRecord.MISSING;
		landed = false;
		boost = false;
		rssi = AltosRecord.MISSING;
		status = 0;
		device_type = AltosRecord.MISSING;
		config_major = AltosRecord.MISSING;
		config_minor = AltosRecord.MISSING;
		apogee_delay = AltosRecord.MISSING;
		main_deploy = AltosRecord.MISSING;
		flight_log_max = AltosRecord.MISSING;

		ground_altitude = new AltosCValue();
		ground_pressure = new AltosGroundPressure();
		altitude = new AltosAltitude();
		pressure = new AltosPressure();
		speed = new AltosSpeed();
		acceleration = new AltosAccel();

		temperature = AltosRecord.MISSING;
		battery_voltage = AltosRecord.MISSING;
		pyro_voltage = AltosRecord.MISSING;
		apogee_voltage = AltosRecord.MISSING;
		main_voltage = AltosRecord.MISSING;
		ignitor_voltage = null;

		kalman_height = new AltosValue();
		kalman_speed = new AltosValue();
		kalman_acceleration = new AltosValue();

		gps = null;
		temp_gps = null;
		temp_gps_clear_sats_pending = false;
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
		ground_accel_avg = AltosRecord.MISSING;

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

		received_time = old.received_time;
		time = old.time;
		time_change = 0;
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

		ground_altitude.copy(old.ground_altitude);
		altitude.copy(old.altitude);
		pressure.copy(old.pressure);
		speed.copy(old.speed);
		acceleration.copy(old.acceleration);

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
		temp_gps_clear_sats_pending = old.temp_gps_clear_sats_pending;
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
		ground_accel_avg = old.ground_accel_avg;

		log_format = old.log_format;
		serial = old.serial;

		baro = old.baro;
		companion = old.companion;
	}
	
	void update_time() {
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
			double h = height();

			if (h == AltosRecord.MISSING)
				h = 0;
			from_pad = new AltosGreatCircle(pad_lat, pad_lon, 0, gps.lat, gps.lon, h);
			elevation = from_pad.elevation;
			range = from_pad.range;
			gps_height = gps.alt - pad_alt;
		}
	}

	public void set_tick(int new_tick) {
		if (new_tick != AltosRecord.MISSING) {
			if (prev_tick != AltosRecord.MISSING) {
				while (new_tick < prev_tick - 32767) {
					new_tick += 65536;
				}
			}
			tick = new_tick;
			time = tick / 100.0;
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

	public void set_device_type(int device_type) {
		this.device_type = device_type;
	}

	public void set_config(int major, int minor, int apogee_delay, int main_deploy, int flight_log_max) {
		config_major = major;
		config_minor = minor;
		this.apogee_delay = apogee_delay;
		this.main_deploy = main_deploy;
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

	void update_accel() {
		double	ground = ground_accel;

		if (ground == AltosRecord.MISSING)
			ground = ground_accel_avg;
		if (accel == AltosRecord.MISSING)
			return;
		if (ground == AltosRecord.MISSING)
			return;
		if (accel_plus_g == AltosRecord.MISSING)
			return;
		if (accel_minus_g == AltosRecord.MISSING)
			return;

		double counts_per_g = (accel_minus_g - accel_plus_g) / 2.0;
		double counts_per_mss = counts_per_g / 9.80665;
		acceleration.set_computed((ground - accel) / counts_per_mss, time);
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
			if (state == AltosLib.ao_flight_pad) {
				if (ground_accel_avg == AltosRecord.MISSING)
					ground_accel_avg = accel;
				else
					ground_accel_avg = (ground_accel_avg * 7 + accel) / 8;
			}
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

	public void set_ignitor_voltage(double[] voltage) {
		this.ignitor_voltage = voltage;
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

	public AltosGPS make_temp_gps(boolean sats) {
		if (temp_gps == null) {
			temp_gps = new AltosGPS(gps);
		}
		gps_pending = true;
		if (!sats)
			temp_gps_clear_sats_pending = true;
		else if (temp_gps_clear_sats_pending) {
			temp_gps.cc_gps_sat = null;
			temp_gps_clear_sats_pending = false;
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

		received_time = System.currentTimeMillis();

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

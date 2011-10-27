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
import java.text.*;
import java.util.HashMap;
import java.io.*;

public class AltosRecord implements Comparable <AltosRecord> {
	final static int	MISSING = 0x7fffffff;

	static final int	seen_flight = 1;
	static final int	seen_sensor = 2;
	static final int	seen_temp_volt = 4;
	static final int	seen_deploy = 8;
	static final int	seen_gps_time = 16;
	static final int	seen_gps_lat = 32;
	static final int	seen_gps_lon = 64;
	static final int	seen_companion = 128;
	int			seen;

	int	version;
	String 	callsign;
	int	serial;
	int	flight;
	int	rssi;
	int	status;
	int	state;
	int	tick;

	int	accel;
	int	pres;
	int	temp;
	int	batt;
	int	drogue;
	int	main;

	int	ground_accel;
	int	ground_pres;
	int	accel_plus_g;
	int	accel_minus_g;

	double	acceleration;
	double	speed;
	double	height;

	int	flight_accel;
	int	flight_vel;
	int	flight_pres;

	AltosGPS	gps;
	boolean		new_gps;

	double	time;	/* seconds since boost */

	int	device_type;
	int	config_major;
	int	config_minor;
	int	apogee_delay;
	int	main_deploy;
	int	flight_log_max;
	String	firmware_version;

	AltosRecordCompanion companion;
	/*
	 * Values for our MP3H6115A pressure sensor
	 *
	 * From the data sheet:
	 *
	 * Pressure range: 15-115 kPa
	 * Voltage at 115kPa: 2.82
	 * Output scale: 27mV/kPa
	 *
	 *
	 * 27 mV/kPa * 2047 / 3300 counts/mV = 16.75 counts/kPa
	 * 2.82V * 2047 / 3.3 counts/V = 1749 counts/115 kPa
	 */

	static final double counts_per_kPa = 27 * 2047 / 3300;
	static final double counts_at_101_3kPa = 1674.0;

	static double
	barometer_to_pressure(double count)
	{
		return ((count / 16.0) / 2047.0 + 0.095) / 0.009 * 1000.0;
	}

	public double raw_pressure() {
		if (pres == MISSING)
			return MISSING;
		return barometer_to_pressure(pres);
	}

	public double filtered_pressure() {
		if (flight_pres == MISSING)
			return MISSING;
		return barometer_to_pressure(flight_pres);
	}

	public double ground_pressure() {
		if (ground_pres == MISSING)
			return MISSING;
		return barometer_to_pressure(ground_pres);
	}

	public double raw_altitude() {
		double p = raw_pressure();
		if (p == MISSING)
			return MISSING;
		return AltosConvert.pressure_to_altitude(p);
	}

	public double ground_altitude() {
		double p = ground_pressure();
		if (p == MISSING)
			return MISSING;
		return AltosConvert.pressure_to_altitude(p);
	}

	public double filtered_altitude() {
		if (height != MISSING && ground_pres != MISSING)
			return height + ground_altitude();

		double	p = filtered_pressure();
		if (p == MISSING)
			return MISSING;
		return AltosConvert.pressure_to_altitude(p);
	}

	public double filtered_height() {
		if (height != MISSING)
			return height;

		double f = filtered_altitude();
		double g = ground_altitude();
		if (f == MISSING || g == MISSING)
			return MISSING;
		return f - g;
	}

	public double raw_height() {
		double r = raw_altitude();
		double g = ground_altitude();

		if (r == MISSING || g == MISSING)
			return height;
		return r - g;
	}

	public double battery_voltage() {
		if (batt == MISSING)
			return MISSING;
		return AltosConvert.cc_battery_to_voltage(batt);
	}

	public double main_voltage() {
		if (main == MISSING)
			return MISSING;
		return AltosConvert.cc_ignitor_to_voltage(main);
	}

	public double drogue_voltage() {
		if (drogue == MISSING)
			return MISSING;
		return AltosConvert.cc_ignitor_to_voltage(drogue);
	}

	/* Value for the CC1111 built-in temperature sensor
	 * Output voltage at 0°C = 0.755V
	 * Coefficient = 0.00247V/°C
	 * Reference voltage = 1.25V
	 *
	 * temp = ((value / 32767) * 1.25 - 0.755) / 0.00247
	 *      = (value - 19791.268) / 32768 * 1.25 / 0.00247
	 */

	static double
	thermometer_to_temperature(double thermo)
	{
		return (thermo - 19791.268) / 32728.0 * 1.25 / 0.00247;
	}

	public double temperature() {
		if (temp == MISSING)
			return MISSING;
		return thermometer_to_temperature(temp);
	}

	double accel_counts_per_mss() {
		double	counts_per_g = Math.abs(accel_minus_g - accel_plus_g) / 2;

		return counts_per_g / 9.80665;
	}

	public double acceleration() {
		if (acceleration != MISSING)
			return acceleration;

		if (ground_accel == MISSING || accel == MISSING)
			return MISSING;
		return (ground_accel - accel) / accel_counts_per_mss();
	}

	public double accel_speed() {
		if (speed != MISSING)
			return speed;
		if (flight_vel == MISSING)
			return MISSING;
		return flight_vel / (accel_counts_per_mss() * 100.0);
	}

	public String state() {
		return Altos.state_name(state);
	}

	public static String gets(FileInputStream s) throws IOException {
		int c;
		String	line = "";

		while ((c = s.read()) != -1) {
			if (c == '\r')
				continue;
			if (c == '\n') {
				return line;
			}
			line = line + (char) c;
		}
		return null;
	}

	public int compareTo(AltosRecord o) {
		return tick - o.tick;
	}

	public AltosRecord(AltosRecord old) {
		version = old.version;
		seen = old.seen;
		callsign = old.callsign;
		serial = old.serial;
		flight = old.flight;
		rssi = old.rssi;
		status = old.status;
		state = old.state;
		tick = old.tick;
		accel = old.accel;
		pres = old.pres;
		temp = old.temp;
		batt = old.batt;
		drogue = old.drogue;
		main = old.main;
		flight_accel = old.flight_accel;
		ground_accel = old.ground_accel;
		flight_vel = old.flight_vel;
		flight_pres = old.flight_pres;
		ground_pres = old.ground_pres;
		accel_plus_g = old.accel_plus_g;
		accel_minus_g = old.accel_minus_g;
		acceleration = old.acceleration;
		speed = old.speed;
		height = old.height;
		gps = new AltosGPS(old.gps);
		new_gps = false;
		companion = old.companion;
	}

	public AltosRecord() {
		version = 0;
		seen = 0;
		callsign = "N0CALL";
		serial = 0;
		flight = 0;
		rssi = 0;
		status = 0;
		state = Altos.ao_flight_startup;
		tick = 0;
		accel = MISSING;
		pres = MISSING;
		temp = MISSING;
		batt = MISSING;
		drogue = MISSING;
		main = MISSING;
		flight_accel = 0;
		ground_accel = 0;
		flight_vel = 0;
		flight_pres = 0;
		ground_pres = 0;
		accel_plus_g = 0;
		accel_minus_g = 0;
		acceleration = MISSING;
		speed = MISSING;
		height = MISSING;
		gps = new AltosGPS();
		new_gps = false;
		companion = null;
	}
}

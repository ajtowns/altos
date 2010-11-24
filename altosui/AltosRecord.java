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

public class AltosRecord {
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
	int	flight_accel;
	int	ground_accel;
	int	flight_vel;
	int	flight_pres;
	int	ground_pres;
	int	accel_plus_g;
	int	accel_minus_g;
	AltosGPS	gps;

	double	time;	/* seconds since boost */

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
		return barometer_to_pressure(pres);
	}

	public double filtered_pressure() {
		return barometer_to_pressure(flight_pres);
	}

	public double ground_pressure() {
		return barometer_to_pressure(ground_pres);
	}

	public double filtered_altitude() {
		return AltosConvert.pressure_to_altitude(filtered_pressure());
	}

	public double raw_altitude() {
		return AltosConvert.pressure_to_altitude(raw_pressure());
	}

	public double ground_altitude() {
		return AltosConvert.pressure_to_altitude(ground_pressure());
	}

	public double filtered_height() {
		return filtered_altitude() - ground_altitude();
	}

	public double raw_height() {
		return raw_altitude() - ground_altitude();
	}

	public double battery_voltage() {
		return AltosConvert.cc_battery_to_voltage(batt);
	}

	public double main_voltage() {
		return AltosConvert.cc_ignitor_to_voltage(main);
	}

	public double drogue_voltage() {
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
		return thermometer_to_temperature(temp);
	}

	double accel_counts_per_mss() {
		double	counts_per_g = Math.abs(accel_minus_g - accel_plus_g) / 2;

		return counts_per_g / 9.80665;
	}
	public double acceleration() {
		return (ground_accel - accel) / accel_counts_per_mss();
	}

	public double accel_speed() {
		double speed = flight_vel / (accel_counts_per_mss() * 100.0);
		return speed;
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

	public AltosRecord(AltosRecord old) {
		version = old.version;
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
		gps = new AltosGPS(old.gps);
	}

	public AltosRecord() {
		version = 0;
		callsign = "N0CALL";
		serial = 0;
		flight = 0;
		rssi = 0;
		status = 0;
		state = Altos.ao_flight_startup;
		tick = 0;
		accel = 0;
		pres = 0;
		temp = 0;
		batt = 0;
		drogue = 0;
		main = 0;
		flight_accel = 0;
		ground_accel = 0;
		flight_vel = 0;
		flight_pres = 0;
		ground_pres = 0;
		accel_plus_g = 0;
		accel_minus_g = 0;
		gps = new AltosGPS();
	}
}

/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

public class AltosRecordTM extends AltosRecord {
	public int	accel;
	public int	pres;
	public int	temp;
	public int	batt;
	public int	drogue;
	public int	main;

	public int      ground_accel;
	public int      ground_pres;
	public int      accel_plus_g;
	public int      accel_minus_g;

	public int	flight_accel;
	public int	flight_vel;
	public int	flight_pres;

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

	public void copy(AltosRecordTM old) {
		super.copy(old);

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
	}

	public AltosRecordTM clone() {
		AltosRecordTM	n = (AltosRecordTM) super.clone();
		n.copy(this);
		return n;
	}

	void make_missing() {
		accel = MISSING;
		pres = MISSING;
		temp = MISSING;
		batt = MISSING;
		drogue = MISSING;
		main = MISSING;

		flight_accel = MISSING;
		flight_vel = MISSING;
		flight_pres = MISSING;

		ground_accel = MISSING;
		ground_pres = MISSING;
		accel_plus_g = MISSING;
		accel_minus_g = MISSING;
	}

	public AltosRecordTM(AltosRecord old) {
		super.copy(old);
		make_missing();
	}

	public AltosRecordTM() {
		super();
		make_missing();
	}
}

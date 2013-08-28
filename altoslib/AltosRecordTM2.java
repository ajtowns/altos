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

package org.altusmetrum.altoslib_1;

public class AltosRecordTM2 extends AltosRecord {

	/* Sensor values */
	public int	accel;
	public int	pres;
	public int	temp;
	
	public int	v_batt;
	public int	sense_a;
	public int	sense_m;

	public int      ground_accel;
	public int      ground_pres;
	public int      accel_plus_g;
	public int      accel_minus_g;

	public int	flight_accel;
	public int	flight_vel;
	public int	flight_pres;

	static double adc(int raw) {
		return raw / 4095.0;
	}

	public double pressure() {
		if (pres != MISSING)
			return pres;
		return MISSING;
	}

	public double ground_pressure() {
		if (ground_pres != MISSING)
			return ground_pres;
		return MISSING;
	}

	public double battery_voltage() {
		if (v_batt != MISSING)
			return 3.3 * adc(v_batt) * (15.0 + 27.0) / 27.0;
		return MISSING;
	}

	static double pyro(int raw) {
		if (raw != MISSING)
			return 3.3 * adc(raw) * (100.0 + 27.0) / 27.0;
		return MISSING;
	}

	public double main_voltage() {
		return pyro(sense_m);
	}

	public double drogue_voltage() {
		return pyro(sense_a);
	}

	public double temperature() {
		if (temp != MISSING)
			return temp / 100.0;
		return MISSING;
	}
	
	double accel_counts_per_mss() {
		double	counts_per_g = Math.abs(accel_minus_g - accel_plus_g) / 2;

		return counts_per_g / 9.80665;
	}

	public double acceleration() {
		if (ground_accel == MISSING || accel == MISSING)
			return MISSING;

		if (accel_minus_g == MISSING || accel_plus_g == MISSING)
			return MISSING;

		return (ground_accel - accel) / accel_counts_per_mss();
	}

	public void copy (AltosRecordTM2 old) {
		super.copy(old);

		accel = old.accel;
		pres = old.pres;
		temp = old.temp;

		v_batt = old.v_batt;
		sense_a = old.sense_a;
		sense_m = old.sense_m;

		ground_accel = old.ground_accel;
		ground_pres = old.ground_pres;
		accel_plus_g = old.accel_plus_g;
		accel_minus_g = old.accel_minus_g;
		
		flight_accel = old.flight_accel;
		flight_vel = old.flight_vel;
		flight_pres = old.flight_pres;
	}

	public AltosRecordTM2 clone() {
		return new AltosRecordTM2(this);
	}

	void make_missing() {

		accel = MISSING;
		pres = MISSING;
		temp = MISSING;

		v_batt = MISSING;
		sense_a = MISSING;
		sense_m = MISSING;

		ground_accel = MISSING;
		ground_pres = MISSING;
		accel_plus_g = MISSING;
		accel_minus_g = MISSING;

		flight_accel = 0;
		flight_vel = 0;
		flight_pres = 0;
	}

	public AltosRecordTM2(AltosRecord old) {
		super.copy(old);
		make_missing();
	}

	public AltosRecordTM2(AltosRecordTM2 old) {
		copy(old);
	}

	public AltosRecordTM2() {
		super();
		make_missing();
	}
}

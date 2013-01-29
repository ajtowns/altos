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

public class AltosRecordMM extends AltosRecord {

	/* Sensor values */
	public int	accel;
	public int	pres;
	public int	temp;
	
	public int	v_batt;
	public int	v_pyro;
	public int	sense[];

	public int      ground_accel;
	public int      ground_pres;
	public int      accel_plus_g;
	public int      accel_minus_g;

	public int	flight_accel;
	public int	flight_vel;
	public int	flight_pres;

	public final static int	num_sense = 6;

	public AltosIMU	imu;
	public AltosMag	mag;

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
		return pyro(sense[1]);
	}

	public double drogue_voltage() {
		return pyro(sense[0]);
	}

	public double temperature() {
		if (temp != MISSING)
			return temp / 100.0;
		return MISSING;
	}
	
	public AltosIMU imu() { return imu; }

	public AltosMag mag() { return mag; }

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

	public void copy (AltosRecordMM old) {
		super.copy(old);

		accel = old.accel;
		pres = old.pres;
		temp = old.temp;

		v_batt = old.v_batt;
		v_pyro = old.v_pyro;
		sense = new int[num_sense];
		
		for (int i = 0; i < num_sense; i++)
			sense[i] = old.sense[i];

		ground_accel = old.ground_accel;
		ground_pres = old.ground_pres;
		accel_plus_g = old.accel_plus_g;
		accel_minus_g = old.accel_minus_g;
		
		flight_accel = old.flight_accel;
		flight_vel = old.flight_vel;
		flight_pres = old.flight_pres;

		imu = old.imu;
		mag = old.mag;
	}



	public AltosRecordMM clone() {
		return new AltosRecordMM(this);
	}

	void make_missing() {

		accel = MISSING;
		pres = MISSING;
		temp = MISSING;

		v_batt = MISSING;
		v_pyro = MISSING;
		sense = new int[num_sense];
		for (int i = 0; i < num_sense; i++)
			sense[i] = MISSING;

		ground_accel = MISSING;
		ground_pres = MISSING;
		accel_plus_g = MISSING;
		accel_minus_g = MISSING;

		flight_accel = 0;
		flight_vel = 0;
		flight_pres = 0;

		imu = new AltosIMU();
		mag = new AltosMag();
	}

	public AltosRecordMM(AltosRecord old) {
		super.copy(old);
		make_missing();
	}

	public AltosRecordMM(AltosRecordMM old) {
		copy(old);
	}

	public AltosRecordMM() {
		super();
		make_missing();
	}
}

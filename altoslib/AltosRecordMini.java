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

public class AltosRecordMini extends AltosRecord {

	/* Sensor values */
	public int	pres;
	public int	temp;
	
	public int	sense_a;
	public int	sense_m;
	public int	v_batt;

	public int      ground_pres;

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

	public double temperature() {
		if (temp != MISSING)
			return temp;
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

	public double apogee_voltage() {
		return pyro(sense_a);
	}

	public void copy (AltosRecordMini old) {
		super.copy(old);

		pres = old.pres;
		temp = old.temp;

		sense_a = old.sense_a;
		sense_m = old.sense_m;
		v_batt = old.v_batt;

		ground_pres = old.ground_pres;
		
		flight_accel = old.flight_accel;
		flight_vel = old.flight_vel;
		flight_pres = old.flight_pres;
	}



	public AltosRecordMini clone() {
		return new AltosRecordMini(this);
	}

	void make_missing() {

		pres = MISSING;

		sense_a = MISSING;
		sense_m = MISSING;
		v_batt = MISSING;

		ground_pres = MISSING;

		flight_accel = 0;
		flight_vel = 0;
		flight_pres = 0;
	}

	public AltosRecordMini(AltosRecord old) {
		super.copy(old);
		make_missing();
	}

	public AltosRecordMini(AltosRecordMini old) {
		copy(old);
	}

	public AltosRecordMini() {
		super();
		make_missing();
	}
}

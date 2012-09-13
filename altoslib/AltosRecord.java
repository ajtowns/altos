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

package org.altusmetrum.AltosLib;

public class AltosRecord implements Comparable <AltosRecord>, Cloneable {

	public static final int	seen_flight = 1;
	public static final int	seen_sensor = 2;
	public static final int	seen_temp_volt = 4;
	public static final int	seen_deploy = 8;
	public static final int	seen_gps_time = 16;
	public static final int	seen_gps_lat = 32;
	public static final int	seen_gps_lon = 64;
	public static final int	seen_companion = 128;

	public int	seen;
	
	public final static int	MISSING = 0x7fffffff;

	/* Every AltosRecord implementation provides these fields */
	
	public int	version;
	public String 	callsign;
	public int	serial;
	public int	flight;
	public int	rssi;
	public int	status;
	public int	state;
	public int	tick;

	/* Current flight dynamic state */
	public double	acceleration;	/* m/s² */
	public double	speed;		/* m/s */
	public double	height;		/* m */

	public AltosGPS	gps;
	public boolean	new_gps;

	public double	time;	/* seconds since boost */

	public int	device_type;
	public int	config_major;
	public int	config_minor;
	public int	apogee_delay;
	public int	main_deploy;
	public int	flight_log_max;
	public String	firmware_version;

	public AltosRecordCompanion companion;

	/*
	 * Abstract methods that convert record data
	 * to standard units:
	 *
	 *	pressure:	Pa
	 *	voltage:	V
	 *	acceleration:	m/s²
	 *	speed:		m/s
	 *	height:		m
	 *	temperature:	°C
	 */

	public double raw_pressure() { return MISSING; }

	public double filtered_pressure() { return MISSING; }

	public double ground_pressure() { return MISSING; }

	public double battery_voltage() { return MISSING; }

	public double main_voltage() { return MISSING; }

	public double drogue_voltage() { return MISSING; }

	public double temperature() { return MISSING; }
	
	public double acceleration() { return MISSING; }

	public double accel_speed() { return MISSING; }

	public AltosIMU imu() { return null; }

	public AltosMag mag() { return null; }

	/*
	 * Convert various pressure values to altitude
	 */

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
		double	ga = ground_altitude();
		if (height != MISSING && ga != MISSING)
			return height + ga;

		double	p = filtered_pressure();
		if (p == MISSING)
			return raw_altitude();
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

	public String state() {
		return AltosLib.state_name(state);
	}

	public int compareTo(AltosRecord o) {
		return tick - o.tick;
	}

	public void copy(AltosRecord old) {
		seen = old.seen;
		version = old.version;
		callsign = old.callsign;
		serial = old.serial;
		flight = old.flight;
		rssi = old.rssi;
		status = old.status;
		state = old.state;
		tick = old.tick;
		acceleration = old.acceleration;
		speed = old.speed;
		height = old.height;
		gps = new AltosGPS(old.gps);
		new_gps = old.new_gps;
		companion = old.companion;
	}

	public AltosRecord clone() {
		try {
			AltosRecord n = (AltosRecord) super.clone();
			n.copy(this);
			return n;
		} catch (CloneNotSupportedException e) {
			return null;
		}
	}

	public AltosRecord() {
		seen = 0;
		version = 0;
		callsign = "N0CALL";
		serial = 0;
		flight = 0;
		rssi = 0;
		status = 0;
		state = AltosLib.ao_flight_startup;
		tick = 0;
		acceleration = MISSING;
		speed = MISSING;
		height = MISSING;
		gps = new AltosGPS();
		new_gps = false;
		companion = null;
	}
}

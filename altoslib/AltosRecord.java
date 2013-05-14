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

package org.altusmetrum.altoslib_1;

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

	public AltosGPS	gps;
	public int	gps_sequence;

	public double	time;	/* seconds since boost */

	public int	device_type;
	public int	config_major;
	public int	config_minor;
	public int	apogee_delay;
	public int	main_deploy;
	public int	flight_log_max;
	public String	firmware_version;

	public AltosRecordCompanion companion;

	/* Telemetry sources have these values recorded from the flight computer */
	public double	kalman_height;
	public double	kalman_speed;
	public double	kalman_acceleration;

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

	public double pressure() { return MISSING; }
	public double ground_pressure() { return MISSING; }
	public double acceleration() { return MISSING; }

	public double altitude() {
		double	p = pressure();

		if (p == MISSING)
			return MISSING;
		return AltosConvert.pressure_to_altitude(p);
	}

	public double ground_altitude() {
		double	p = ground_pressure();

		if (p == MISSING)
			return MISSING;
		return AltosConvert.pressure_to_altitude(p);
	}

	public double height() {
		double	g = ground_altitude();
		double	a = altitude();

		if (g == MISSING)
			return MISSING;
		if (a == MISSING)
			return MISSING;
		return a - g;
	}

	public double battery_voltage() { return MISSING; }

	public double main_voltage() { return MISSING; }

	public double drogue_voltage() { return MISSING; }

	public double temperature() { return MISSING; }
	
	public AltosIMU imu() { return null; }

	public AltosMag mag() { return null; }

	public String state() {
		return AltosLib.state_name(state);
	}

	public int compareTo(AltosRecord o) {
		return tick - o.tick;
	}

	public AltosRecord clone() {
		AltosRecord n = new AltosRecord();
		n.copy(this);
		return n;
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
		gps = new AltosGPS(old.gps);
		gps_sequence = old.gps_sequence;
		companion = old.companion;
		kalman_acceleration = old.kalman_acceleration;
		kalman_speed = old.kalman_speed;
		kalman_height = old.kalman_height;
	}

	public AltosRecord() {
		seen = 0;
		version = 0;
		callsign = "N0CALL";
		serial = MISSING;
		flight = MISSING;
		rssi = 0;
		status = 0;
		state = AltosLib.ao_flight_startup;
		tick = 0;
		gps = null;
		gps_sequence = 0;
		companion = null;

		kalman_acceleration = MISSING;
		kalman_speed = MISSING;
		kalman_height = MISSING;
	}
}

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

package org.altusmetrum.AltosLib;

public class AltosState {
	public AltosRecord data;

	/* derived data */

	public long  	report_time;

	public double	time;
	public double	time_change;
	public int	tick;

	public int	state;
	public boolean	landed;
	public boolean	ascent;	/* going up? */
	public boolean boost;	/* under power */

	public double	ground_altitude;
	public double	altitude;
	public double	height;
	public double	speed;
	public double	acceleration;
	public double	battery;
	public double	temperature;
	public double	main_sense;
	public double	drogue_sense;
	public double	baro_speed;

	public double	max_height;
	public double	max_acceleration;
	public double	max_speed;
	public double	max_baro_speed;

	public AltosGPS	gps;

	public AltosIMU	imu;
	public AltosMag	mag;

	public static final int MIN_PAD_SAMPLES = 10;

	public int	npad;
	public int	ngps;
	public int	gps_waiting;
	public boolean	gps_ready;

	public AltosGreatCircle from_pad;
	public double	elevation;	/* from pad */
	public double	range;		/* total distance */

	public double	gps_height;

	public double pad_lat, pad_lon, pad_alt;

	public int	speak_tick;
	public double	speak_altitude;

	public void init (AltosRecord cur, AltosState prev_state) {
		//int		i;
		//AltosRecord prev;

		data = cur;

		ground_altitude = data.ground_altitude();
		altitude = data.raw_altitude();
		height = data.filtered_height();

		report_time = System.currentTimeMillis();

		acceleration = data.acceleration();
		speed = data.accel_speed();
		temperature = data.temperature();
		drogue_sense = data.drogue_voltage();
		main_sense = data.main_voltage();
		battery = data.battery_voltage();
		tick = data.tick;
		state = data.state;

		if (prev_state != null) {

			/* Preserve any existing gps data */
			npad = prev_state.npad;
			ngps = prev_state.ngps;
			gps = prev_state.gps;
			pad_lat = prev_state.pad_lat;
			pad_lon = prev_state.pad_lon;
			pad_alt = prev_state.pad_alt;
			max_height = prev_state.max_height;
			max_acceleration = prev_state.max_acceleration;
			max_speed = prev_state.max_speed;
			max_baro_speed = prev_state.max_baro_speed;
			imu = prev_state.imu;
			mag = prev_state.mag;

			/* make sure the clock is monotonic */
			while (tick < prev_state.tick)
				tick += 65536;

			time_change = (tick - prev_state.tick) / 100.0;

			/* compute barometric speed */

			double height_change = height - prev_state.height;
			if (data.speed != AltosRecord.MISSING)
				baro_speed = data.speed;
			else {
				if (time_change > 0)
					baro_speed = (prev_state.baro_speed * 3 + (height_change / time_change)) / 4.0;
				else
					baro_speed = prev_state.baro_speed;
			}
		} else {
			npad = 0;
			ngps = 0;
			gps = null;
			baro_speed = 0;
			time_change = 0;
		}

		time = tick / 100.0;

		if (cur.new_gps && (state == AltosLib.ao_flight_pad || state == AltosLib.ao_flight_idle)) {

			/* Track consecutive 'good' gps reports, waiting for 10 of them */
			if (data.gps != null && data.gps.locked && data.gps.nsat >= 4)
				npad++;
			else
				npad = 0;

			/* Average GPS data while on the pad */
			if (data.gps != null && data.gps.locked && data.gps.nsat >= 4) {
				if (ngps > 1) {
					/* filter pad position */
					pad_lat = (pad_lat * 31.0 + data.gps.lat) / 32.0;
					pad_lon = (pad_lon * 31.0 + data.gps.lon) / 32.0;
					pad_alt = (pad_alt * 31.0 + data.gps.alt) / 32.0;
				} else {
					pad_lat = data.gps.lat;
					pad_lon = data.gps.lon;
					pad_alt = data.gps.alt;
				}
				ngps++;
			}
		} else
			pad_alt = ground_altitude;

		data.new_gps = false;

		gps_waiting = MIN_PAD_SAMPLES - npad;
		if (gps_waiting < 0)
			gps_waiting = 0;

		gps_ready = gps_waiting == 0;

		ascent = (AltosLib.ao_flight_boost <= state &&
			  state <= AltosLib.ao_flight_coast);
		boost = (AltosLib.ao_flight_boost == state);

		/* Only look at accelerometer data under boost */
		if (boost && acceleration > max_acceleration && acceleration != AltosRecord.MISSING)
			max_acceleration = acceleration;
		if (boost && speed > max_speed && speed != AltosRecord.MISSING)
			max_speed = speed;
		if (boost && baro_speed > max_baro_speed && baro_speed != AltosRecord.MISSING)
			max_baro_speed = baro_speed;

		if (height > max_height && height != AltosRecord.MISSING)
			max_height = height;
		if (data.gps != null) {
			if (gps == null || !gps.locked || data.gps.locked)
				gps = data.gps;
			if (ngps > 0 && gps.locked) {
				from_pad = new AltosGreatCircle(pad_lat, pad_lon, gps.lat, gps.lon);
			}
		}
		elevation = 0;
		range = -1;
		if (ngps > 0) {
			gps_height = gps.alt - pad_alt;
			if (from_pad != null) {
				elevation = Math.atan2(height, from_pad.distance) * 180 / Math.PI;
				range = Math.sqrt(height * height + from_pad.distance * from_pad.distance);
			}
		} else {
			gps_height = 0;
		}
	}

	public AltosState(AltosRecord cur) {
		init(cur, null);
	}

	public AltosState (AltosRecord cur, AltosState prev) {
		init(cur, prev);
	}
}

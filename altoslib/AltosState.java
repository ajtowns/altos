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
	public double	acceleration;
	public double	battery;
	public double	temperature;
	public double	main_sense;
	public double	drogue_sense;
	public double	accel_speed;
	public double	baro_speed;

	public double	max_height;
	public double	max_acceleration;
	public double	max_accel_speed;
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

	public double speed() {
		if (ascent)
			return accel_speed;
		else
			return baro_speed;
	}

	public double max_speed() {
		if (max_accel_speed != 0)
			return max_accel_speed;
		return max_baro_speed;
	}

	public void init (AltosRecord cur, AltosState prev_state) {
		data = cur;

		/* Discard previous state if it was for a different board */
		if (prev_state != null && prev_state.data.serial != data.serial)
			prev_state = null;
		ground_altitude = data.ground_altitude();

		altitude = data.altitude();
		if (altitude == AltosRecord.MISSING && data.gps != null)
			altitude = data.gps.alt;

		height = AltosRecord.MISSING;
		if (data.kalman_height != AltosRecord.MISSING)
			height = data.kalman_height;
		else {
			if (altitude != AltosRecord.MISSING && ground_altitude != AltosRecord.MISSING) {
				double	cur_height = altitude - ground_altitude;
				if (prev_state == null || prev_state.height == AltosRecord.MISSING)
					height = cur_height;
				else
					height = (prev_state.height * 15 + cur_height) / 16.0;
			}
		}

		report_time = System.currentTimeMillis();

		if (data.kalman_acceleration != AltosRecord.MISSING)
			acceleration = data.kalman_acceleration;
		else
			acceleration = data.acceleration();
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
			max_accel_speed = prev_state.max_accel_speed;
			max_baro_speed = prev_state.max_baro_speed;
			imu = prev_state.imu;
			mag = prev_state.mag;

			/* make sure the clock is monotonic */
			while (tick < prev_state.tick)
				tick += 65536;

			time_change = (tick - prev_state.tick) / 100.0;

			if (data.kalman_speed != AltosRecord.MISSING) {
				baro_speed = accel_speed = data.kalman_speed;
			} else {
				/* compute barometric speed */

				double height_change = height - prev_state.height;

				double prev_baro_speed = prev_state.baro_speed;
				if (prev_baro_speed == AltosRecord.MISSING)
					prev_baro_speed = 0;

				if (time_change > 0)
					baro_speed = (prev_baro_speed * 3 + (height_change / time_change)) / 4.0;
				else
					baro_speed = prev_state.baro_speed;

				double prev_accel_speed = prev_state.accel_speed;

				if (prev_accel_speed == AltosRecord.MISSING)
					prev_accel_speed = 0;

				if (acceleration == AltosRecord.MISSING) {
					/* Fill in mising acceleration value */
					accel_speed = baro_speed;

					if (time_change > 0 && accel_speed != AltosRecord.MISSING)
						acceleration = (accel_speed - prev_accel_speed) / time_change;
					else
						acceleration = prev_state.acceleration;
				} else {
					/* compute accelerometer speed */
					accel_speed = prev_accel_speed + acceleration * time_change;
				}
			}
		} else {
			npad = 0;
			ngps = 0;
			gps = null;
			baro_speed = AltosRecord.MISSING;
			accel_speed = AltosRecord.MISSING;
			pad_alt = AltosRecord.MISSING;
			max_baro_speed = 0;
			max_accel_speed = 0;
			max_height = 0;
			max_acceleration = 0;
			time_change = 0;
		}

		time = tick / 100.0;

		if (cur.new_gps && (state < AltosLib.ao_flight_boost)) {

			/* Track consecutive 'good' gps reports, waiting for 10 of them */
			if (data.gps != null && data.gps.locked && data.gps.nsat >= 4)
				npad++;
			else
				npad = 0;

			/* Average GPS data while on the pad */
			if (data.gps != null && data.gps.locked && data.gps.nsat >= 4) {
				if (ngps > 1 && state == AltosLib.ao_flight_pad) {
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
		} else {
			if (ngps == 0 && ground_altitude != AltosRecord.MISSING)
				pad_alt = ground_altitude;
		}

		data.new_gps = false;

		gps_waiting = MIN_PAD_SAMPLES - npad;
		if (gps_waiting < 0)
			gps_waiting = 0;

		gps_ready = gps_waiting == 0;

		ascent = (AltosLib.ao_flight_boost <= state &&
			  state <= AltosLib.ao_flight_coast);
		boost = (AltosLib.ao_flight_boost == state);

		/* Only look at accelerometer data under boost */
		if (boost && acceleration != AltosRecord.MISSING && (max_acceleration == AltosRecord.MISSING || acceleration > max_acceleration))
			max_acceleration = acceleration;
		if (boost && accel_speed != AltosRecord.MISSING && accel_speed > max_accel_speed)
			max_accel_speed = accel_speed;
		if (boost && baro_speed != AltosRecord.MISSING && baro_speed > max_baro_speed)
			max_baro_speed = baro_speed;

		if (height != AltosRecord.MISSING && height > max_height)
			max_height = height;
		elevation = 0;
		range = -1;
		gps_height = 0;
		if (data.gps != null) {
			if (gps == null || !gps.locked || data.gps.locked)
				gps = data.gps;
			if (ngps > 0 && gps.locked) {
				double h = height;

				if (h == AltosRecord.MISSING) h = 0;
				from_pad = new AltosGreatCircle(pad_lat, pad_lon, 0, gps.lat, gps.lon, h);
				elevation = from_pad.elevation;
				range = from_pad.range;
				gps_height = gps.alt - pad_alt;
			}
		}
	}

	public AltosState(AltosRecord cur) {
		init(cur, null);
	}

	public AltosState (AltosRecord cur, AltosState prev) {
		init(cur, prev);
	}
}

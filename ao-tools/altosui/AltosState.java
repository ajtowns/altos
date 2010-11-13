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

package altosui;

public class AltosState {
	AltosRecord data;

	/* derived data */

	long  	report_time;

	double	time_change;
	int	tick;

	int	state;
	boolean	landed;
	boolean	ascent;	/* going up? */

	double	ground_altitude;
	double	height;
	double	speed;
	double	acceleration;
	double	battery;
	double	temperature;
	double	main_sense;
	double	drogue_sense;
	double	baro_speed;

	double	max_height;
	double	max_acceleration;
	double	max_speed;

	AltosGPS	gps;

	double	pad_lat;
	double	pad_lon;
	double	pad_alt;

	static final int MIN_PAD_SAMPLES = 10;

	int	npad;
	int	ngps;
	int	gps_waiting;
	boolean	gps_ready;

	AltosGreatCircle from_pad;
	double	elevation;	/* from pad */
	double	range;		/* total distance */

	double	gps_height;

	int	speak_tick;
	double	speak_altitude;


	void init (AltosRecord cur, AltosState prev_state) {
		int		i;
		AltosRecord prev;

		data = cur;

		ground_altitude = data.ground_altitude();
		height = data.filtered_altitude() - ground_altitude;

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

			/* make sure the clock is monotonic */
			while (tick < prev_state.tick)
				tick += 65536;

			time_change = (tick - prev_state.tick) / 100.0;

			/* compute barometric speed */

			double height_change = height - prev_state.height;
			if (time_change > 0)
				baro_speed = (prev_state.baro_speed * 3 + (height_change / time_change)) / 4.0;
			else
				baro_speed = prev_state.baro_speed;
		} else {
			npad = 0;
			ngps = 0;
			gps = null;
			baro_speed = 0;
			time_change = 0;
		}

		if (state == Altos.ao_flight_pad) {

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
		}

		gps_waiting = MIN_PAD_SAMPLES - npad;
		if (gps_waiting < 0)
			gps_waiting = 0;

		gps_ready = gps_waiting == 0;

		ascent = (Altos.ao_flight_boost <= state &&
			  state <= Altos.ao_flight_coast);

		/* Only look at accelerometer data on the way up */
		if (ascent && acceleration > max_acceleration)
			max_acceleration = acceleration;
		if (ascent && speed > max_speed)
			max_speed = speed;

		if (height > max_height)
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

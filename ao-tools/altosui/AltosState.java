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
 * Track flight state from telemetry data stream
 */

package altosui;

import altosui.AltosTelemetry;
import altosui.AltosGPS;

public class AltosState {
	AltosTelemetry data;

	/* derived data */

	double  report_time;

	double	time_change;
	int	tick;

	int	state;
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
	int	npad;
	int	prev_npad;

	AltosGreatCircle from_pad;

	double	gps_height;

	int	speak_tick;
	double	speak_altitude;

	static double
	aoview_time()
	{
		return System.currentTimeMillis() / 1000.0;
	}

	void init (AltosTelemetry cur, AltosState prev_state) {
		int		i;
		AltosTelemetry prev;
		double	accel_counts_per_mss;

		data = cur;

		ground_altitude = AltosConvert.cc_barometer_to_altitude(data.ground_pres);
		height = AltosConvert.cc_barometer_to_altitude(data.flight_pres) - ground_altitude;

		report_time = aoview_time();

		accel_counts_per_mss = ((data.accel_minus_g - data.accel_plus_g) / 2.0) / 9.80665;
		acceleration = (data.ground_accel - data.flight_accel) / accel_counts_per_mss;
		speed = data.flight_vel / (accel_counts_per_mss * 100.0);
		temperature = AltosConvert.cc_thermometer_to_temperature(data.temp);
		drogue_sense = AltosConvert.cc_ignitor_to_voltage(data.drogue);
		main_sense = AltosConvert.cc_ignitor_to_voltage(data.main);
		battery = AltosConvert.cc_battery_to_voltage(data.batt);
		tick = data.tick;
		state = data.state();

		if (prev_state != null) {

			/* Preserve any existing gps data */
			npad = prev_state.npad;
			gps = prev_state.gps;
			pad_lat = prev_state.pad_lat;
			pad_lon = prev_state.pad_lon;
			pad_alt = prev_state.pad_alt;

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
			gps = null;
			baro_speed = 0;
			time_change = 0;
		}

		if (state == AltosTelemetry.ao_flight_pad) {
			if (data.gps != null && data.gps.gps_locked && data.gps.nsat >= 4) {
				npad++;
				if (npad > 1) {
					/* filter pad position */
					pad_lat = (pad_lat * 31.0 + data.gps.lat) / 32.0;
					pad_lon = (pad_lon * 31.0 + data.gps.lon) / 32.0;
					pad_alt = (pad_alt * 31.0 + data.gps.alt) / 32.0;
				} else {
					pad_lat = data.gps.lat;
					pad_lon = data.gps.lon;
					pad_alt = data.gps.alt;
				}
			}
		}
		ascent = (AltosTelemetry.ao_flight_boost <= state &&
			  state <= AltosTelemetry.ao_flight_coast);

		/* Only look at accelerometer data on the way up */
		if (ascent && acceleration > max_acceleration)
			max_acceleration = acceleration;
		if (ascent && speed > max_speed)
			max_speed = speed;

		if (height > max_height)
			max_height = height;
		if (data.gps != null) {
			if (gps == null || !gps.gps_locked || data.gps.gps_locked)
				gps = data.gps;
			if (npad > 0 && gps.gps_locked)
				from_pad = new AltosGreatCircle(pad_lat, pad_lon, gps.lat, gps.lon);
		}
		if (npad > 0) {
			gps_height = gps.alt - pad_alt;
		} else {
			gps_height = 0;
		}
	}

	public AltosState(AltosTelemetry cur) {
		init(cur, null);
	}

	public AltosState (AltosTelemetry cur, AltosState prev) {
		init(cur, prev);
	}
}

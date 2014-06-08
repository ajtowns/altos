/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

package org.altusmetrum.altoslib_4;


public class AltosTelemetryConfiguration extends AltosTelemetryStandard {
	int	device_type;
	int	flight;
	int	config_major;
	int	config_minor;
	int	apogee_delay;
	int	main_deploy;
	int	v_batt;
	int	flight_log_max;
	String	callsign;
	String	version;

	public AltosTelemetryConfiguration(int[] bytes) {
		super(bytes);

		device_type    = uint8(5);
		flight         = uint16(6);
		config_major   = uint8(8);
		config_minor   = uint8(9);
		v_batt	       = uint16(10);
		apogee_delay   = uint16(10);
		main_deploy    = uint16(12);
		flight_log_max = uint16(14);
		callsign       = string(16, 8);
		version        = string(24, 8);
	}

	public void update_state(AltosState state) {
		super.update_state(state);
		state.set_device_type(device_type);
		state.set_flight(flight);
		state.set_config(config_major, config_minor, flight_log_max);
		if (device_type == AltosLib.product_telegps)
			state.set_battery_voltage(AltosConvert.tele_gps_voltage(v_batt));
		else
			state.set_flight_params(apogee_delay, main_deploy);

		state.set_callsign(callsign);
		state.set_firmware_version(version);
	}
}

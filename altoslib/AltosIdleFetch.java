/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

import java.io.*;
import java.util.*;
import java.text.*;
import java.util.concurrent.*;

class AltosIdler {
	String	prefix;
	int[]	idlers;

	static final int	idle_gps = 0;
	static final int	idle_imu = 1;
	static final int	idle_mag = 2;
	static final int	idle_ms5607 = 3;
	static final int	idle_mma655x = 4;


	static final int	idle_sensor_tm = 10;
	static final int	idle_sensor_metrum = 11;
	static final int	idle_sensor_mega = 12;
	static final int	idle_sensor_emini = 13;
	static final int	idle_sensor_tmini = 14;

	public void update_state(AltosState state, AltosLink link, AltosConfigData config_data) throws InterruptedException, TimeoutException {
		for (int idler : idlers) {
			AltosIdle idle = null;
			switch (idler) {
			case idle_gps:
				AltosGPS.update_state(state, link, config_data);
				break;
			case idle_imu:
				AltosIMU.update_state(state, link, config_data);
				break;
			case idle_mag:
				AltosMag.update_state(state, link, config_data);
				break;
			case idle_ms5607:
				AltosMs5607.update_state(state, link, config_data);
				break;
			case idle_mma655x:
				AltosMma655x.update_state(state, link, config_data);
				break;
			case idle_sensor_tm:
				AltosSensorTM.update_state(state, link, config_data);
				break;
			case idle_sensor_metrum:
				AltosSensorMetrum.update_state(state, link, config_data);
				break;
			case idle_sensor_mega:
				AltosSensorMega.update_state(state, link, config_data);
				break;
			case idle_sensor_emini:
				AltosSensorEMini.update_state(state, link, config_data);
				break;
			case idle_sensor_tmini:
				AltosSensorTMini.update_state(state, link, config_data);
			}
			if (idle != null)
				idle.update_state(state);
		}
	}

	public boolean matches(AltosConfigData config_data) {
		return config_data.product.startsWith(prefix);
	}

	public AltosIdler(String prefix, int ... idlers) {
		this.prefix = prefix;
		this.idlers = idlers;
	}
}


public class AltosIdleFetch implements AltosStateUpdate {

	static final AltosIdler[] idlers = {

		new AltosIdler("EasyMini",
			       AltosIdler.idle_ms5607,
			       AltosIdler.idle_sensor_emini),

		new AltosIdler("TeleMini-v1",
			       AltosIdler.idle_sensor_tm),

		new AltosIdler("TeleMini-v2",
			       AltosIdler.idle_ms5607,
			       AltosIdler.idle_sensor_tmini),

		new AltosIdler("TeleMetrum-v1",
			       AltosIdler.idle_gps,
			       AltosIdler.idle_sensor_tm),

		new AltosIdler("TeleMetrum-v2",
			       AltosIdler.idle_gps,
			       AltosIdler.idle_ms5607, AltosIdler.idle_mma655x,
			       AltosIdler.idle_sensor_metrum),

		new AltosIdler("TeleMega",
			       AltosIdler.idle_gps,
			       AltosIdler.idle_ms5607, AltosIdler.idle_mma655x,
			       AltosIdler.idle_imu, AltosIdler.idle_mag,
			       AltosIdler.idle_sensor_mega),
	};

	AltosLink		link;

	double			frequency;
	String			callsign;

	public void update_state(AltosState state) throws InterruptedException {
		try {
			AltosConfigData config_data = new AltosConfigData(link);
			state.set_state(AltosLib.ao_flight_startup);
			state.set_serial(config_data.serial);
			state.set_callsign(config_data.callsign);
			state.set_ground_accel(config_data.accel_cal_plus);
			state.set_accel_g(config_data.accel_cal_plus, config_data.accel_cal_minus);
			for (AltosIdler idler : idlers) {
				if (idler.matches(config_data)) {
					idler.update_state(state, link, config_data);
					break;
				}
			}
			state.set_received_time(System.currentTimeMillis());
		} catch (TimeoutException te) {
		}

	}

	public AltosIdleFetch(AltosLink link) {
		this.link = link;
	}
}

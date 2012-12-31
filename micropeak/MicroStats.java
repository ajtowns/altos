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

package org.altusmetrum.micropeak;

import java.io.*;
import org.altusmetrum.AltosLib.*;
import org.altusmetrum.altosuilib.*;

public class MicroStats {
	double		coast_height;
	double		coast_time;

	double		apogee_height;
	double		apogee_time;

	double		landed_height;
	double		landed_time;

	double		max_speed;
	double		max_accel;

	MicroData	data;

	void find_landing() {
		landed_height = 0;

		int t = 0;
		for (double height : data.heights()) {
			landed_height = height;
			t++;
		}
		landed_time = data.time(t);

		t = 0;
		boolean above = false;
		for (double height : data.heights()) {
			if (height > landed_height + 10) {
				above = true;
			} else {
				if (above && height < landed_height + 2) {
					above = false;
					landed_time = data.time(t);
				}
			}
			t++;
		}
	}

	void find_apogee() {
		apogee_height = 0;
		apogee_time = 0;
		
		int t = 0;
		for (double height : data.heights()) {
			if (height > apogee_height) {
				apogee_height = height;
				apogee_time = data.time(t);
			}
			t++;
		}
	}

	void find_coast() {
		coast_height = 0;
		coast_time = 0;

		int t = 0;
		for (double accel : data.accels()) {
			if (accel < -9.8)
				break;
			t++;
		}
		coast_time = data.time(t);

		int coast_t = t;
		t = 0;
		for (double height : data.heights()) {
			if (t >= coast_t) {
				coast_height = height;
				break;
			}
			t++;
		}
	}

	void find_max_speed() {
		max_speed = 0;
		int	t = 0;
		for (double speed : data.speeds()) {
			if (data.time(t) > apogee_time)
				break;
			if (speed > max_speed)
				max_speed = speed;
			t++;
		}
	}

	void find_max_accel() {
		max_accel = 0;

		int t = 0;
		for (double accel : data.accels()) {
			if (data.time(t) > apogee_time)
				break;
			if (accel > max_accel)
				max_accel = accel;
			t++;
		}
	}

	double boost_duration() {
		return coast_time;
	}

	double boost_height() {
		return coast_height;
	}

	double	boost_speed() {
		return coast_height / coast_time;
	}

	double boost_accel() {
		return boost_speed() / boost_duration();
	}

	double coast_duration() {
		return apogee_time - coast_time;
	}

	double coast_height() {
		return apogee_height - coast_height;
	}

	double coast_speed() {
		return coast_height() / coast_duration();
	}

	double coast_accel() {
		return coast_speed() / coast_duration();
	}

	double descent_duration() {
		return landed_time - apogee_time;
	}

	double descent_height() {
		return apogee_height - landed_height;
	}

	double descent_speed() {
		return descent_height() / descent_duration();
	}

	public MicroStats(MicroData data) {

		this.data = data;

		find_coast();
		find_apogee();
		find_landing();
		find_max_speed();
		find_max_accel();
	}

	public MicroStats() {
		this(new MicroData());
	}
}

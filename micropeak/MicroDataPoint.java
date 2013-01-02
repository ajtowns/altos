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

package org.altusmetrum.micropeak;

public class MicroDataPoint {
	public double	time;
	public double	pressure;
	public double	height;
	public double	speed;
	public double	accel;

	public MicroDataPoint (double pressure, double height, double speed, double accel, double time) {
		this.pressure = pressure;
		this.height = height;
		this.speed = speed;
		this.accel = accel;
		this.time = time;
	}

	public MicroDataPoint(MicroData data, int i) {
		this(data.pressure(i),
		     data.height(i),
		     data.speed(i),
		     data.acceleration(i),
		     data.time(i));
	}
}
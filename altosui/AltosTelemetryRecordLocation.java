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

package altosui;


public class AltosTelemetryRecordLocation extends AltosTelemetryRecordRaw {
	int	flags;
	int	altitude;
	int	latitude;
	int	longitude;
	int	year;
	int	month;
	int	day;
	int	hour;
	int	minute;
	int	second;
	int	pdop;
	int	hdop;
	int	vdop;
	int	mode;
	int	ground_speed;
	int	climb_rate;
	int	course;

	public AltosTelemetryRecordLocation(int[] in_bytes) {
		super(in_bytes);

		flags          = uint8(5);
		altitude       = int16(6);
		latitude       = uint32(8);
		longitude      = uint32(12);
		year	       = uint8(16);
		month	       = uint8(17);
		day	       = uint8(18);
		hour	       = uint8(19);
		minute	       = uint8(20);
		second	       = uint8(21);
		pdop	       = uint8(22);
		hdop	       = uint8(23);
		vdop	       = uint8(24);
		mode	       = uint8(25);
		ground_speed   = uint16(26);
		climb_rate     = int16(28);
		course	       = uint8(30);
	}

	public AltosRecord update_state(AltosRecord previous) {
		AltosRecord	next = super.update_state(previous);

		if (next.gps == null)
			next.gps = new AltosGPS();

		next.gps.nsat = flags & 0xf;
		next.gps.locked = (flags & (1 << 4)) != 0;
		next.gps.connected = (flags & (1 << 5)) != 0;

		if (next.gps.locked) {
			next.gps.lat = latitude * 1.0e-7;
			next.gps.lon = longitude * 1.0e-7;
			next.gps.alt = altitude;
			next.gps.year = 2000 + year;
			next.gps.month = month;
			next.gps.day = day;
			next.gps.hour = hour;
			next.gps.minute = minute;
			next.gps.second = second;
			next.gps.ground_speed = ground_speed * 1.0e-2;
			next.gps.course = course * 2;
			next.gps.climb_rate = climb_rate * 1.0e-2;
			next.gps.hdop = hdop;
			next.gps.vdop = vdop;
			next.seen |= AltosRecord.seen_gps_time | AltosRecord.seen_gps_lat | AltosRecord.seen_gps_lon;
			next.new_gps = true;
		}

		return next;
	}
}

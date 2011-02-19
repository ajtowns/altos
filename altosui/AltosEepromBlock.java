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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.table.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.util.concurrent.*;
import java.lang.reflect.Array;

import libaltosJNI.*;

public class AltosEepromBlock extends ArrayList<AltosEepromRecord> {
	boolean	has_flight;
	int	flight;
	boolean	has_state;
	int	state;
	boolean	has_date;
	int	year, month, day;
	boolean	has_lat;
	double	lat;
	boolean	has_lon;
	double	lon;
	boolean	has_time;
	int	hour, minute, second;
	ParseException	parse_exception = null;

	public AltosEepromBlock (AltosSerial serial_line, int block) throws TimeoutException, InterruptedException {
		int	addr;
		boolean	done = false;

		has_flight = false;
		has_state = false;
		has_date = false;
		has_lat = false;
		has_lon = false;
		has_time = false;
		serial_line.printf("e %x\n", block);
		for (addr = 0; addr < 0x100;) {
			try {
				AltosEepromRecord r = new AltosEepromRecord(serial_line, block * 256 + addr);

				if (r.cmd == Altos.AO_LOG_FLIGHT) {
					flight = r.b;
					has_flight = true;
				}

				/* Monitor state transitions to update display */
				if (r.cmd == Altos.AO_LOG_STATE && r.a <= Altos.ao_flight_landed) {
					if (!has_state || r.a > state) {
						state = r.a;
						has_state = true;
					}
				}

				if (r.cmd == Altos.AO_LOG_GPS_DATE) {
					year = 2000 + (r.a & 0xff);
					month = (r.a >> 8) & 0xff;
					day = (r.b & 0xff);
					has_date = true;
				}
				if (r.cmd == Altos.AO_LOG_GPS_TIME) {
					hour = (r.a & 0xff);
					minute = (r.a >> 8);
					second = (r.b & 0xff);
					has_time = true;
				}
				if (r.cmd == Altos.AO_LOG_GPS_LAT) {
					lat = (double) (r.a | (r.b << 16)) / 1e7;
					has_lat = true;
				}
				if (r.cmd == Altos.AO_LOG_GPS_LON) {
					lon = (double) (r.a | (r.b << 16)) / 1e7;
					has_lon = true;
				}
				if (!done)
					add(addr / 8, r);
				if (r.cmd == Altos.AO_LOG_STATE && r.a == Altos.ao_flight_landed)
					done = true;
			} catch (ParseException pe) {
				AltosEepromRecord	r = new AltosEepromRecord(Altos.AO_LOG_INVALID,
										  0, 0, 0);
				if (parse_exception == null)
					parse_exception = pe;
				if (!done)
					add(addr/8, r);
			}
			addr += 8;
		}
	}
}
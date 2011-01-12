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

import libaltosJNI.*;

/*
 * Extract a bit of information from an eeprom-stored flight log.
 */

public class AltosEepromLog {
	int		serial;
	boolean		has_flight;
	int		flight;
	int		start_block;
	int		end_block;

	boolean		has_gps;
	int		year, month, day;
	int		hour, minute, second;
	double		lat, lon;

	boolean		download;
	boolean		delete;

	public AltosEepromLog(AltosSerial serial_line, int in_serial,
			      int in_start_block, int in_end_block)
		throws InterruptedException, TimeoutException {

		int		block;
		boolean		has_date = false, has_time = false, has_lat = false, has_lon = false;

		start_block = in_start_block;
		end_block = in_end_block;
		serial = in_serial;

		/*
		 * By default, request that every log be downloaded but not deleted
		 */
		download = true;
		delete = false;
		/*
		 * Only look in the first two blocks so that this
		 * process doesn't take a long time
		 */
		if (in_end_block > in_start_block + 2)
			in_end_block = in_start_block + 2;

		for (block = in_start_block; block < in_end_block; block++) {
			AltosEepromBlock eeblock = new AltosEepromBlock(serial_line, block);
			if (eeblock.has_flight) {
				flight = eeblock.flight;
				has_flight = true;
			}
			if (eeblock.has_date) {
				year = eeblock.year;
				month = eeblock.month;
				day = eeblock.day;
				has_date = true;
			}
			if (eeblock.has_time) {
				hour = eeblock.hour;
				minute = eeblock.minute;
				second = eeblock.second;
				has_time = true;
			}
			if (eeblock.has_lat) {
				lat = eeblock.lat;
				has_lat = true;
			}
			if (eeblock.has_lon) {
				lon = eeblock.lon;
				has_lon = true;
			}
			if (has_date && has_time && has_lat && has_lon)
				has_gps = true;
			if (has_gps && has_flight)
				break;
		}
		System.out.printf("Serial %d start block %d end block %d\n",
				  serial, start_block, end_block);
		if (has_flight)
			System.out.printf("Flight %d\n", flight);
		if (has_gps)
			System.out.printf("%d-%d-%d %d:%02d:%02d Lat %f Lon %f\n",
					  year, month, day, hour, minute, second, lat, lon);
	}
}

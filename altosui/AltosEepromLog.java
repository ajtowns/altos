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

	int		year, month, day;

	boolean		download;
	boolean		delete;

	public AltosEepromLog(AltosSerial serial_line, int in_serial,
			      int in_start_block, int in_end_block)
		throws InterruptedException, TimeoutException {

		int		block;
		boolean		has_date = false;

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
			AltosEepromChunk eechunk = new AltosEepromChunk(serial_line, block);

			if (block == in_start_block) {
				if (eechunk.data(0) != Altos.AO_LOG_FLIGHT) {
					flight = eechunk.data16(0);
					has_flight = true;
					break;
				}
			}
			for (int i = 0; i < eechunk.chunk_size; i += AltosEepromRecord.record_length) {
				try {
					AltosEepromRecord r = new AltosEepromRecord(eechunk, i);

					if (r.cmd == Altos.AO_LOG_FLIGHT) {
						flight = r.b;
						has_flight = true;
					}
					if (r.cmd == Altos.AO_LOG_GPS_DATE) {
						year = 2000 + (r.a & 0xff);
						month = (r.a >> 8) & 0xff;
						day = (r.b & 0xff);
						has_date = true;
					}
				} catch (ParseException pe) {
				}
			}
			if (has_date && has_flight)
				break;
		}
	}
}

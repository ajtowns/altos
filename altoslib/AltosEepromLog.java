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

import java.text.*;
import java.util.concurrent.*;

/*
 * Extract a bit of information from an eeprom-stored flight log.
 */

public class AltosEepromLog {
	public int		serial;
	public boolean		has_flight;
	public int		flight;
	public int		start_block;
	public int		end_block;

	public int		year, month, day;

	public boolean		selected;

	public AltosEepromLog(AltosConfigData config_data,
			      AltosLink link,
			      int in_flight, int in_start_block,
			      int in_end_block)
		throws InterruptedException, TimeoutException {

		int		block;
		boolean		has_date = false;

		flight = in_flight;
		if (flight != 0)
			has_flight = true;
		start_block = in_start_block;
		end_block = in_end_block;
		serial = config_data.serial;

		/*
		 * Select all flights for download
		 */
		selected = true;

		/*
		 * Look in TeleMetrum log data for date
		 */
		if (config_data.log_format == AltosLib.AO_LOG_FORMAT_UNKNOWN ||
		    config_data.log_format == AltosLib.AO_LOG_FORMAT_FULL)
		{
			/*
			 * Only look in the first two blocks so that this
			 * process doesn't take a long time
			 */
			if (in_end_block > in_start_block + 2)
				in_end_block = in_start_block + 2;

			for (block = in_start_block; block < in_end_block; block++) {
				AltosEepromChunk eechunk = new AltosEepromChunk(link, block, block == in_start_block);

				for (int i = 0; i < AltosEepromChunk.chunk_size; i += AltosEepromTM.record_length) {
					try {
						AltosEepromTM r = new AltosEepromTM(eechunk, i);

						if (r.cmd == AltosLib.AO_LOG_FLIGHT) {
							flight = r.b;
							has_flight = true;
						}
						if (r.cmd == AltosLib.AO_LOG_GPS_DATE) {
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
}

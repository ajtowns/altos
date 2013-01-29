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

package org.altusmetrum.altoslib_1;

import java.text.*;

public class AltosTelemetryRecordGeneral {

	static AltosTelemetryRecord parse(String line) throws ParseException, AltosCRCException {
		AltosTelemetryRecord	r;

		String[] word = line.split("\\s+");
		int i =0;
		if (word[i].equals("CRC") && word[i+1].equals("INVALID")) {
			i += 2;
			AltosParse.word(word[i++], "RSSI");
			throw new AltosCRCException(AltosParse.parse_int(word[i++]));
		}

		if (word[i].equals("TELEM"))
			r = AltosTelemetryRecordRaw.parse(word[i+1]);
		else
			r = new AltosTelemetryRecordLegacy(line);
		return r;
	}
}

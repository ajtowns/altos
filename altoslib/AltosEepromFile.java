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

package org.altusmetrum.altoslib_1;

import java.io.*;
import java.util.*;
import java.text.*;

public class AltosEepromFile {

	AltosEepromIterable	headers;
	AltosEepromIterable	body;

	public void write(PrintStream out) {
		headers.write(out);
		body.write(out);
	}

	public AltosEepromFile(FileInputStream input) {
		headers = new AltosEepromIterable(AltosEepromHeader.read(input));

		AltosState	state = headers.state();

		switch (state.log_format) {
		case AltosLib.AO_LOG_FORMAT_FULL:
		case AltosLib.AO_LOG_FORMAT_TINY:
		case AltosLib.AO_LOG_FORMAT_TELEMETRY:
		case AltosLib.AO_LOG_FORMAT_TELESCIENCE:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA:
			break;
		case AltosLib.AO_LOG_FORMAT_MINI:
			body = new AltosEepromIterable(AltosEepromMini.read(input));
			break;
		}
	}
}
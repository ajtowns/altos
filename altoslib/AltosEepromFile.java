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

class AltosEepromIterator implements Iterator<AltosState> {
	AltosState		state;
	Iterator<AltosEeprom>	body;
	AltosEeprom		next;
	boolean			seen;

	public boolean hasNext() {
		return !seen || body.hasNext();
	}

	public AltosState next() {
		if (seen) {
			AltosState	n = state.clone();
			AltosEeprom	e = body.next();

			e.update_state(n);
			state = n;
		}
		seen = true;
		return state;
	}

	public void remove () {
	}

	public AltosEepromIterator(AltosState start, Iterator<AltosEeprom> body) {
		this.state = start;
		this.body = body;
		this.seen = false;
	}
}

public class AltosEepromFile extends AltosStateIterable {

	AltosEepromIterable	headers;
	AltosEepromIterable	body;
	AltosState		start;

	public void write_comments(PrintStream out) {
		headers.write(out);
	}

	public void write(PrintStream out) {
		headers.write(out);
		body.write(out);
	}

	public AltosEepromFile(FileInputStream input) {
		headers = new AltosEepromIterable(AltosEepromHeader.read(input));

		start = headers.state();
		if (start.state != AltosLib.ao_flight_stateless)
			start.set_state(AltosLib.ao_flight_pad);

		if (start.log_format == AltosLib.MISSING) {
			if (start.product != null) {
				if (start.product.startsWith("TeleMetrum"))
					start.log_format = AltosLib.AO_LOG_FORMAT_FULL;
				else if (start.product.startsWith("TeleMini"))
					start.log_format = AltosLib.AO_LOG_FORMAT_TINY;
			}
		}

		switch (start.log_format) {
		case AltosLib.AO_LOG_FORMAT_FULL:
			body = new AltosEepromIterable(AltosEepromTM.read(input));
			break;
		case AltosLib.AO_LOG_FORMAT_TINY:
			body = new AltosEepromIterable(AltosEepromTm.read(input));
			break;
		case AltosLib.AO_LOG_FORMAT_TELEMETRY:
		case AltosLib.AO_LOG_FORMAT_TELESCIENCE:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA:
			body = new AltosEepromIterable(AltosEepromMega.read(input));
			break;
		case AltosLib.AO_LOG_FORMAT_TELEMETRUM:
			body = new AltosEepromIterable(AltosEepromMetrum2.read(input));
			break;
		case AltosLib.AO_LOG_FORMAT_TELEMINI:
		case AltosLib.AO_LOG_FORMAT_EASYMINI:
			body = new AltosEepromIterable(AltosEepromMini.read(input));
			break;
		case AltosLib.AO_LOG_FORMAT_TELEGPS:
			body = new AltosEepromIterable(AltosEepromGPS.read(input));
			break;
		default:
			body = new AltosEepromIterable(new LinkedList<AltosEeprom>());
			break;
		}

		/* Find boost tick */
		AltosState	state = start.clone();
		for (AltosEeprom eeprom : body) {
			eeprom.update_state(state);
			state.finish_update();
			if (state.state >= AltosLib.ao_flight_boost) {
				start.set_boost_tick(state.tick);
				break;
			}
		}
	}

	public Iterator<AltosState> iterator() {
		AltosState		state = start.clone();
		Iterator<AltosEeprom>	i = body.iterator();

		while (i.hasNext() && !state.valid()) {
			i.next().update_state(state);
			state.finish_update();
		}
		return new AltosEepromIterator(state, i);
	}
}

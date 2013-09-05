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

package org.altusmetrum.altoslib_2;

import java.io.*;
import java.util.*;
import java.text.*;

public class AltosEepromHeaderIterable implements Iterable<AltosEepromHeader> {
	public LinkedList<AltosEepromHeader> headers;

	public void write(PrintStream out) {
		AltosEepromHeader.write(out, headers);
	}

	public AltosState state() {
		AltosState	state = new AltosState();

		for (AltosEepromHeader header : headers)
			header.update_state(state);
		return state;
	}

	public AltosEepromHeaderIterable(FileInputStream input) {
		headers = AltosEepromHeader.read(input);
	}

	public Iterator<AltosEepromHeader> iterator() {
		if (headers == null)
			headers = new LinkedList<AltosEepromHeader>();
		return headers.iterator();
	}
}
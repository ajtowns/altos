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

class AltosTelemetryIterator implements Iterator<AltosState> {
	AltosState			state;
	Iterator<AltosTelemetry>	telems;
	AltosTelemetry			next;
	boolean				seen;

	public boolean hasNext() {
		return !seen || telems.hasNext();
	}

	public AltosState next() {
		if (seen) {
			AltosState	n = state.clone();
			AltosTelemetry	t = telems.next();

			t.update_state(n);
			state = n;
		}
		seen = true;
		return state;
	}

	public void remove () {
	}

	public AltosTelemetryIterator(AltosState start, Iterator<AltosTelemetry> telems) {
		this.state = start;
		this.telems = telems;
		this.seen = false;
	}
}

public class AltosTelemetryFile extends AltosStateIterable {

	AltosTelemetryIterable	telems;
	AltosState		start;

	public void write_comments(PrintStream out) {
	}

	public void write(PrintStream out) {

	}

	public AltosTelemetryFile(FileInputStream input) {
		telems = new AltosTelemetryIterable(input);
		start = new AltosState();

		/* Find boost tick */
		AltosState	state = start.clone();

		for (AltosTelemetry telem : telems) {
			telem.update_state(state);
			state.finish_update();
			if (state.state != AltosLib.ao_flight_invalid && state.state >= AltosLib.ao_flight_boost) {
				start.set_boost_tick(state.tick);
				break;
			}
		}
	}

	public Iterator<AltosState> iterator() {
		AltosState			state = start.clone();
		Iterator<AltosTelemetry>  	i = telems.iterator();

		while (i.hasNext() && !state.valid()) {
			AltosTelemetry	t = i.next();
			t.update_state(state);
			state.finish_update();
		}
		return new AltosTelemetryIterator(state, i);
	}
}
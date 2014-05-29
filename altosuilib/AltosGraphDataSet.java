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

package org.altusmetrum.altosuilib_2;

import java.lang.*;
import java.io.*;
import java.util.*;
import org.altusmetrum.altoslib_4.*;

class AltosGraphIterator implements Iterator<AltosUIDataPoint> {
	AltosGraphDataSet	dataSet;
	Iterator<AltosState>	iterator;

	public boolean hasNext() {
		return iterator.hasNext();
	}

	public AltosUIDataPoint next() {
		AltosState	state = iterator.next();

		if (state.flight != AltosLib.MISSING) {
			if (dataSet.callsign == null && state.callsign != null)
				dataSet.callsign = state.callsign;

			if (dataSet.serial == 0 && state.serial != 0)
				dataSet.serial = state.serial;

			if (dataSet.flight == 0 && state.flight != 0)
				dataSet.flight = state.flight;
		}

		return new AltosGraphDataPoint(state);
	}

	public AltosGraphIterator (Iterator<AltosState> iterator, AltosGraphDataSet dataSet) {
		this.iterator = iterator;
		this.dataSet = dataSet;
	}

	public void remove() {
	}
}

class AltosGraphIterable implements Iterable<AltosUIDataPoint> {
	AltosGraphDataSet	dataSet;

	public Iterator<AltosUIDataPoint> iterator() {
		return new AltosGraphIterator(dataSet.states.iterator(), dataSet);
	}

	public AltosGraphIterable(AltosGraphDataSet dataSet) {
		this.dataSet = dataSet;
	}
}

public class AltosGraphDataSet implements AltosUIDataSet {
	String			callsign;
	int			serial;
	int			flight;
	AltosStateIterable	states;

	public String name() {
		if (callsign != null)
			return String.format("%s - %d/%d", callsign, serial, flight);
		else
			return String.format("%d/%d", serial, flight);
	}

	public Iterable<AltosUIDataPoint> dataPoints() {
		return new AltosGraphIterable(this);
	}

	public AltosGraphDataSet (AltosStateIterable states) {
		this.states = states;
		this.callsign = null;
		this.serial = 0;
		this.flight = 0;
	}
}

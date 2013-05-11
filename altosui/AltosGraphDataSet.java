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

package altosui;

import java.lang.*;
import java.io.*;
import java.util.*;
import org.altusmetrum.altoslib_1.*;
import org.altusmetrum.altosuilib_1.*;

class AltosGraphIterator implements Iterator<AltosUIDataPoint> {
	AltosGraphDataSet	dataSet;
	Iterator<AltosRecord>	iterator;

	AltosState	state;

	public boolean hasNext() {
		return iterator.hasNext();
	}

	public AltosUIDataPoint next() {
		state = new AltosState(iterator.next(), state);

		if ((state.data.seen & AltosRecord.seen_flight) != 0) {
			if (dataSet.callsign == null && state.data.callsign != null)
				dataSet.callsign = state.data.callsign;

			if (dataSet.serial == 0 && state.data.serial != 0)
				dataSet.serial = state.data.serial;

			if (dataSet.flight == 0 && state.data.flight != 0)
				dataSet.flight = state.data.flight;
		}

		return new AltosGraphDataPoint(state);
	}

	public AltosGraphIterator (Iterator<AltosRecord> iterator, AltosGraphDataSet dataSet) {
		this.iterator = iterator;
		this.state = null;
		this.dataSet = dataSet;
	}

	public void remove() {
	}
}

class AltosGraphIterable implements Iterable<AltosUIDataPoint> {
	AltosGraphDataSet	dataSet;

	public Iterator<AltosUIDataPoint> iterator() {
		return new AltosGraphIterator(dataSet.records.iterator(), dataSet);
	}

	public AltosGraphIterable(AltosGraphDataSet dataSet) {
		this.dataSet = dataSet;
	}
}

public class AltosGraphDataSet implements AltosUIDataSet {
	String			callsign;
	int			serial;
	int			flight;
	AltosRecordIterable	records;

	public String name() {
		if (callsign != null)
			return String.format("%s - %d/%d", callsign, serial, flight);
		else
			return String.format("%d/%d", serial, flight);
	}

	public Iterable<AltosUIDataPoint> dataPoints() {
		return new AltosGraphIterable(this);
	}

	public AltosGraphDataSet (AltosRecordIterable records) {
		this.records = records;
		this.callsign = null;
		this.serial = 0;
		this.flight = 0;
	}
}

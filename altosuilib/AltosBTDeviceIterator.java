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

package org.altusmetrum.altosuilib_2;

import java.util.*;
import libaltosJNI.*;
import org.altusmetrum.altoslib_4.*;

public class AltosBTDeviceIterator implements Iterator<AltosBTDevice> {
	AltosBTDevice	current;
	boolean		done;
	SWIGTYPE_p_altos_bt_list list;

	public boolean hasNext() {
		if (list == null)
			return false;
		if (current != null)
			return true;
		if (done)
			return false;
		current = new AltosBTDevice();
		while (libaltos.altos_bt_list_next(list, current) != 0) {
//			if (current.matchProduct(product))
				return true;
		}
		current = null;
		done = true;
		return false;
	}

	public AltosBTDevice next() {
		if (hasNext()) {
			AltosBTDevice	next = current;
			current = null;
			return next;
		}
		return null;
	}

	public void remove() {
		throw new UnsupportedOperationException();
	}

	public AltosBTDeviceIterator(int inquiry_time) {
		done = false;
		current = null;
		list = libaltos.altos_bt_list_start(inquiry_time);
	}
}

/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

import java.util.concurrent.*;

public class AltosMma655x implements Cloneable {

	int	accel;

	public boolean parse_line(String line) {
		String[] items = line.split("\\s+");
		if (line.startsWith("MMA655X value:")) {
			if (items.length >= 3)
				accel = Integer.parseInt(items[1]);
		} else
			return false;
		return true;
	}

	public AltosMma655x() {
		accel = AltosLib.MISSING;
	}

	public AltosMma655x clone() {
		AltosMma655x	n = new AltosMma655x();

		n.accel = accel;
		return n;
	}

	static public void update_state(AltosState state, AltosLink link, AltosConfigData config_data) throws InterruptedException {
		try {
			AltosMma655x	mma655x = new AltosMma655x(link);

			if (mma655x != null)
				state.set_accel(mma655x.accel);
		} catch (TimeoutException te) {
		}
	}

	public AltosMma655x(AltosLink link) throws InterruptedException, TimeoutException {
		this();
		link.printf("A\n");
		for (;;) {
			String line = link.get_reply_no_dialog(5000);
			if (line == null)
				throw new TimeoutException();
			if (!parse_line(line))
				break;
		}
	}
}

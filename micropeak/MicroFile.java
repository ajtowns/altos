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

package org.altusmetrum.micropeak;

import java.io.*;
import java.util.*;
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

public class MicroFile {

	public static File make(File directory, int year, int month, int day) {
		for (int sequence = 1;; sequence++) {
			String s = String.format("%04d-%02d-%02d-flight-%03d.mpd",
						 year, month, day, sequence);
			File file = new File(directory, s);
			if (!file.exists())
				return file;
		}
	}

	public static File make(File directory) {
		Calendar	cal = Calendar.getInstance();
		return make(directory, cal.get(Calendar.YEAR), cal.get(Calendar.MONTH) + 1, cal.get(Calendar.DAY_OF_MONTH));
	}

	public static File make() {
		return make(AltosUIPreferences.logdir());
	}
}
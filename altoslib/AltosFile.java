/*
 * Copyright © 2010 Keith Packard <keithp@keithp.com>
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

import java.io.File;
import java.util.*;

public class AltosFile extends File {

	static String number(int n) {
		if (n == AltosLib.MISSING)
			return "unkn";
		else
			return String.format("%04d", n);
	}

	static String receiver(int receiver) {
		if (receiver == AltosLib.MISSING)
			return "";
		return String.format("-via-%04d", receiver);
	}

	public AltosFile(int year, int month, int day, int serial, int flight, int receiver, String extension) {
		super (AltosPreferences.logdir(),
		       String.format("%04d-%02d-%02d-serial-%s-flight-%s%s.%s",
				     year, month, day, number(serial), number(flight), receiver(receiver), extension));
	}

	public AltosFile(int year, int month, int day, int serial, int flight, String extension) {
		this(year, month, day, serial, flight, AltosLib.MISSING, extension);
	}

	public AltosFile(int serial, int flight, int receiver, String extension) {
		this(Calendar.getInstance().get(Calendar.YEAR),
		     Calendar.getInstance().get(Calendar.MONTH) + 1,
		     Calendar.getInstance().get(Calendar.DAY_OF_MONTH),
		     serial,
		     flight,
		     receiver,
		     extension);
	}

	public AltosFile(int serial, int flight, String extension) {
		this(Calendar.getInstance().get(Calendar.YEAR),
		     Calendar.getInstance().get(Calendar.MONTH) + 1,
		     Calendar.getInstance().get(Calendar.DAY_OF_MONTH),
		     serial,
		     flight,
		     AltosLib.MISSING,
		     extension);
	}

	public AltosFile(AltosState state) {
		this(state.serial, state.flight, state.receiver_serial, "telem");
	}
}

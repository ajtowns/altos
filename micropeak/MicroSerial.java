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

package org.altusmetrum.micropeak;

import java.util.*;
import java.io.*;
import libaltosJNI.*;
import org.altusmetrum.altosuilib_1.*;

public class MicroSerial extends InputStream {
	SWIGTYPE_p_altos_file	file;

	public int read() {
		int	c = libaltos.altos_getchar(file, 0);
		if (Thread.interrupted())
			return -1;
		if (c == -1)
			return -1;
		if (AltosUIPreferences.serial_debug)
			System.out.printf("%c", c);
		return c;
	}

	public void close() {
		if (file != null) {
			libaltos.altos_close(file);
			file = null;
		}
	}

	public MicroSerial(AltosDevice device) throws FileNotFoundException {
		file = device.open();
		if (file == null) {
			final String message = device.getErrorString();
			throw new FileNotFoundException(String.format("%s (%s)",
								      device.toShortString(),
								      message));
		}
	}
}

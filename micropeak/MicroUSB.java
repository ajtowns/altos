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

package org.altusmetrum.micropeak;

import java.util.*;
import libaltosJNI.*;
import org.altusmetrum.altosuilib_2.*;

public class MicroUSB extends altos_device implements AltosDevice {

	static boolean	initialized = false;
	static boolean	loaded_library = false;

	public static boolean load_library() {
		if (!initialized) {
			try {
				System.loadLibrary("altos");
				libaltos.altos_init();
				loaded_library = true;
			} catch (UnsatisfiedLinkError e) {
				try {
					System.loadLibrary("altos64");
					libaltos.altos_init();
					loaded_library = true;
				} catch (UnsatisfiedLinkError e2) {
					loaded_library = false;
				}
			}
			initialized = true;
		}
		return loaded_library;
	}

	public String toString() {
		String	name = getName();
		if (name == null)
			name = "Altus Metrum";
		return String.format("%-24.24s %s",
				     name, getPath());
	}

	public String toShortString() {
		String	name = getName();
		if (name == null)
			name = "Altus Metrum";
		return String.format("%s %s",
				     name, getPath());

	}

	public String getErrorString() {
		altos_error	error = new altos_error();

		libaltos.altos_get_last_error(error);
		return String.format("%s (%d)", error.getString(), error.getCode());
	}

	public SWIGTYPE_p_altos_file open() {
		return libaltos.altos_open(this);
	}

	private boolean isMicro() {
		if (getVendor() != 0x0403)
			return false;
		if (getProduct() != 0x6015)
			return false;
		return true;
	}

	public boolean matchProduct(int product) {
		return isMicro();
	}

	static java.util.List<MicroUSB> list() {
		if (!load_library())
			return null;

		SWIGTYPE_p_altos_list list = libaltos.altos_ftdi_list_start();

		ArrayList<MicroUSB> device_list = new ArrayList<MicroUSB>();
		if (list != null) {
			for (;;) {
				MicroUSB device = new MicroUSB();
				if (libaltos.altos_list_next(list, device) == 0)
					break;
				if (device.isMicro())
					device_list.add(device);
			}
			libaltos.altos_list_finish(list);
		}

		return device_list;
	}
}
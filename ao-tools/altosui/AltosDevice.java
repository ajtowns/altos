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

package altosui;
import java.lang.*;
import java.util.*;
import libaltosJNI.*;

public class AltosDevice extends altos_device {

	public String toString() {
		return String.format("%-20.20s %4d %s",
				     getProduct(), getSerial(), getPath());
	}

	static {
		System.loadLibrary("altos");
		libaltos.altos_init();
	}
	static AltosDevice[] list(String product) {
		SWIGTYPE_p_altos_list list = libaltos.altos_list_start();

		ArrayList<AltosDevice> device_list = new ArrayList<AltosDevice>();
		if (list != null) {
			SWIGTYPE_p_altos_file file;

			for (;;) {
				AltosDevice device = new AltosDevice();
				if (libaltos.altos_list_next(list, device) == 0)
					break;
				device_list.add(device);
			}
			libaltos.altos_list_finish(list);
		}

		AltosDevice[] devices = new AltosDevice[device_list.size()];
		for (int i = 0; i < device_list.size(); i++)
			devices[i] = device_list.get(i);
		return devices;
	}
}
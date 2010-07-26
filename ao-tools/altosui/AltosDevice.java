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

public class AltosDevice {

	static {
		System.loadLibrary("altos");
		libaltos.altos_init();
	}
	static altos_device[] list(String product) {
		SWIGTYPE_p_altos_list list = libaltos.altos_list_start();

		ArrayList<altos_device> device_list = new ArrayList<altos_device>();
		if (list != null) {
			SWIGTYPE_p_altos_file file;

			for (;;) {
				altos_device device = new altos_device();
				if (libaltos.altos_list_next(list, device) == 0)
					break;
				System.out.printf("Found device %s %d %s\n",
						  device.getProduct(), device.getSerial(), device.getPath());

				device_list.add(device);
			}
			libaltos.altos_list_finish(list);
		}

		altos_device[] devices = new altos_device[device_list.size()];
		for (int i = 0; i < device_list.size(); i++)
			devices[i] = device_list.get(i);
		return devices;
	}
}
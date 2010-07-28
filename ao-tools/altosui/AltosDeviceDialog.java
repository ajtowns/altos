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
import javax.swing.*;
import libaltosJNI.libaltos;
import libaltosJNI.altos_device;
import libaltosJNI.SWIGTYPE_p_altos_file;
import libaltosJNI.SWIGTYPE_p_altos_list;
import altosui.AltosDevice;

public class AltosDeviceDialog {

	static altos_device show (JFrame frame, String product) {
		AltosDevice[]	devices;
		devices = AltosDevice.list(product);
		if (devices != null & devices.length > 0) {
			Object o = JOptionPane.showInputDialog(frame,
							       "Select a device",
							       "Device Selection",
							       JOptionPane.PLAIN_MESSAGE,
							       null,
							       devices,
							       devices[0]);
			return (altos_device) o;
		} else {
			return null;
		}
	}
}

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

import libaltosJNI.*;
import org.altusmetrum.altoslib_4.*;

public class AltosBTDevice extends altos_bt_device implements AltosDevice {

	public String getProductName() {
		String	name = getName();
		if (name == null)
			return "Altus Metrum";
		int	dash = name.lastIndexOf("-");
		if (dash < 0)
			return name;
		return name.substring(0,dash);
	}

	public int getProduct() {
		if (AltosLib.bt_product_telebt.equals(getProductName()))
			return AltosLib.product_telebt;
		return 0;
	}

	public String getPath() {
		return getAddr();
	}

	public String getErrorString() {
		altos_error	error = new altos_error();

		libaltos.altos_get_last_error(error);
		return String.format("%s (%d)", error.getString(), error.getCode());
	}

	public int getSerial() {
		String name = getName();
		if (name == null)
			return 0;
		int dash = name.lastIndexOf("-");
		if (dash < 0 || dash >= name.length())
			return 0;
		String sn = name.substring(dash + 1, name.length());
		try {
			return Integer.parseInt(sn);
		} catch (NumberFormatException ne) {
			return 0;
		}
	}

	public String toString() {
		return String.format("%-20.20s %4d %s",
				     getProductName(), getSerial(), getAddr());
	}

	public String toShortString() {
		return String.format("%s %d %s",
				     getProductName(), getSerial(), getAddr());

	}

	public SWIGTYPE_p_altos_file open() {
		return libaltos.altos_bt_open(this);
	}

	/*
	private boolean isAltusMetrum() {
		if (getName().startsWith(Altos.bt_product_telebt))
			return true;
		return false;
	}
	*/

	public boolean matchProduct(int want_product) {

//		if (!isAltusMetrum())
//			return false;

		if (want_product == AltosLib.product_any)
			return true;

		if (want_product == AltosLib.product_basestation)
			return matchProduct(AltosLib.product_telebt);

		if (want_product == getProduct())
			return true;

		return false;
	}

	public boolean equals(Object o) {
		if (!(o instanceof AltosBTDevice))
			return false;
		AltosBTDevice other = (AltosBTDevice) o;
		return getName().equals(other.getName()) && getAddr().equals(other.getAddr());
	}

	public int hashCode() {
		return getName().hashCode() ^ getAddr().hashCode();
	}

	public AltosBTDevice(String name, String addr) {
		AltosUILib.load_library();
		libaltos.altos_bt_fill_in(name, addr,this);
	}

	public AltosBTDevice() {
	}
}

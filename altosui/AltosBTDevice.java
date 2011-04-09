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

package altosui;
import java.lang.*;
import java.util.*;
import libaltosJNI.*;

public class AltosBTDevice extends altos_bt_device {

	static public boolean initialized = false;
	static public boolean loaded_library = false;

	public static boolean load_library() {
		if (!initialized) {
			try {
				System.loadLibrary("altos");
				libaltos.altos_init();
				loaded_library = true;
			} catch (UnsatisfiedLinkError e) {
				loaded_library = false;
			}
			initialized = true;
		}
		return loaded_library;
	}

	static String bt_product_telebt() {
		if (load_library())
			return libaltosConstants.BLUETOOTH_PRODUCT_TELEBT;
		return "TeleBT";
	}

	public final static String bt_product_telebt = bt_product_telebt();
	public final static String bt_product_any = "Any";
	public final static String bt_product_basestation = "Basestation";

	public String getProduct() {
		String	name = getName();
		if (name == null)
			return "Altus Metrum";
		int	dash = name.lastIndexOf("-");
		if (dash < 0)
			return name;
		return name.substring(0,dash);
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
		String	name = getName();
		if (name == null)
			name = "Altus Metrum";
		return String.format("%-20.20s %4d %s",
				     getProduct(), getSerial(), getAddr());
	}

	public String toShortString() {
		String	name = getName();
		if (name == null)
			name = "Altus Metrum";
		return String.format("%s %d %s",
				     getProduct(), getSerial(), getAddr());

	}

	public boolean isAltusMetrum() {
		if (getName().startsWith(bt_product_telebt))
			return true;
		return false;
	}

	public boolean matchProduct(String want_product) {

		if (!isAltusMetrum())
			return false;

		if (want_product.equals(bt_product_any))
			return true;

		if (want_product.equals(bt_product_basestation))
			return matchProduct(bt_product_telebt);

		if (want_product.equals(getProduct()))
			return true;

		return false;
	}
}
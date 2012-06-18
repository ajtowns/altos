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

public class AltosUSBDevice  extends altos_device implements AltosDevice {

	public String toString() {
		String	name = getName();
		if (name == null)
			name = "Altus Metrum";
		return String.format("%-20.20s %4d %s",
				     name, getSerial(), getPath());
	}

	public String toShortString() {
		String	name = getName();
		if (name == null)
			name = "Altus Metrum";
		return String.format("%s %d %s",
				     name, getSerial(), getPath());

	}

	public String getErrorString() {
		altos_error	error = new altos_error();

		libaltos.altos_get_last_error(error);
		return String.format("%s (%d)", error.getString(), error.getCode());
	}

	public SWIGTYPE_p_altos_file open() {
		return libaltos.altos_open(this);
	}

	private boolean isAltusMetrum() {
		if (getVendor() != Altos.vendor_altusmetrum)
			return false;
		if (getProduct() < Altos.product_altusmetrum_min)
			return false;
		if (getProduct() > Altos.product_altusmetrum_max)
			return false;
		return true;
	}

	public boolean matchProduct(int want_product) {

		if (!isAltusMetrum())
			return false;

		if (want_product == Altos.product_any)
			return true;

		if (want_product == Altos.product_basestation)
			return matchProduct(Altos.product_teledongle) ||
				matchProduct(Altos.product_teleterra) ||
				matchProduct(Altos.product_telebt) ||
				matchProduct(Altos.product_megadongle);

		if (want_product == Altos.product_altimeter)
			return matchProduct(Altos.product_telemetrum) ||
				matchProduct(Altos.product_megametrum);

		int have_product = getProduct();

		if (have_product == Altos.product_altusmetrum)	/* old devices match any request */
			return true;

		if (want_product == have_product)
			return true;

		return false;
	}

	static java.util.List<AltosDevice> list(int product) {
		if (!Altos.load_library())
			return null;

		SWIGTYPE_p_altos_list list = libaltos.altos_list_start();

		ArrayList<AltosDevice> device_list = new ArrayList<AltosDevice>();
		if (list != null) {
			for (;;) {
				AltosUSBDevice device = new AltosUSBDevice();
				if (libaltos.altos_list_next(list, device) == 0)
					break;
				if (device.matchProduct(product))
					device_list.add(device);
			}
			libaltos.altos_list_finish(list);
		}

		return device_list;
	}
}
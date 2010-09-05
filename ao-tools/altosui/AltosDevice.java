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

	static boolean initialized = false;
	static {
		try {
			System.loadLibrary("altos");
			libaltos.altos_init();
			initialized = true;
		} catch (UnsatisfiedLinkError e) {
			System.err.println("Native library failed to load.\n" + e);
		}
	}
	public final static int AltusMetrum = libaltosConstants.USB_PRODUCT_ALTUSMETRUM;
	public final static int TeleMetrum = libaltosConstants.USB_PRODUCT_TELEMETRUM;
	public final static int TeleDongle = libaltosConstants.USB_PRODUCT_TELEDONGLE;
	public final static int TeleTerra = libaltosConstants.USB_PRODUCT_TELETERRA;
	public final static int Any = 0x10000;
	public final static int BaseStation = 0x10000 + 1;

	public String toString() {
		String	name = getName();
		if (name == null)
			name = "Altus Metrum";
		return String.format("%-20.20s %4d %s",
				     getName(), getSerial(), getPath());
	}

	public boolean isAltusMetrum() {
		if (getVendor() != libaltosConstants.USB_VENDOR_ALTUSMETRUM)
			return false;
		if (getProduct() < libaltosConstants.USB_PRODUCT_ALTUSMETRUM_MIN)
			return false;
		if (getProduct() > libaltosConstants.USB_PRODUCT_ALTUSMETRUM_MAX)
			return false;
		return true;
	}

	public boolean matchProduct(int want_product) {

		if (!isAltusMetrum())
			return false;

		if (want_product == Any)
			return true;

		if (want_product == BaseStation)
			return matchProduct(TeleDongle) || matchProduct(TeleTerra);

		int have_product = getProduct();

		if (have_product == AltusMetrum)	/* old devices match any request */
			return true;

		if (want_product == have_product)
			return true;

		return false;
	}

	static AltosDevice[] list(int product) {
		if (!initialized)
			return null;

		SWIGTYPE_p_altos_list list = libaltos.altos_list_start();

		ArrayList<AltosDevice> device_list = new ArrayList<AltosDevice>();
		if (list != null) {
			SWIGTYPE_p_altos_file file;

			for (;;) {
				AltosDevice device = new AltosDevice();
				if (libaltos.altos_list_next(list, device) == 0)
					break;
				if (device.matchProduct(product))
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
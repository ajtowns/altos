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

	static int usb_vendor_altusmetrum() {
		if (load_library())
			return libaltosConstants.USB_VENDOR_ALTUSMETRUM;
		return 0x000a;
	}

	static int usb_product_altusmetrum() {
		if (load_library())
			return libaltosConstants.USB_PRODUCT_ALTUSMETRUM;
		return 0x000a;
	}

	static int usb_product_altusmetrum_min() {
		if (load_library())
			return libaltosConstants.USB_PRODUCT_ALTUSMETRUM_MIN;
		return 0x000a;
	}

	static int usb_product_altusmetrum_max() {
		if (load_library())
			return libaltosConstants.USB_PRODUCT_ALTUSMETRUM_MAX;
		return 0x000d;
	}

	static int usb_product_telemetrum() {
		if (load_library())
			return libaltosConstants.USB_PRODUCT_TELEMETRUM;
		return 0x000b;
	}

	static int usb_product_teledongle() {
		if (load_library())
			return libaltosConstants.USB_PRODUCT_TELEDONGLE;
		return 0x000c;
	}

	static int usb_product_teleterra() {
		if (load_library())
			return libaltosConstants.USB_PRODUCT_TELETERRA;
		return 0x000d;
	}

	public final static int vendor_altusmetrum = usb_vendor_altusmetrum();
	public final static int product_altusmetrum = usb_product_altusmetrum();
	public final static int product_telemetrum = usb_product_telemetrum();
	public final static int product_teledongle = usb_product_teledongle();
	public final static int product_teleterra = usb_product_teleterra();
	public final static int product_altusmetrum_min = usb_product_altusmetrum_min();
	public final static int product_altusmetrum_max = usb_product_altusmetrum_max();


	public final static int product_any = 0x10000;
	public final static int product_basestation = 0x10000 + 1;

	public String toString() {
		String	name = getName();
		if (name == null)
			name = "Altus Metrum";
		return String.format("%-20.20s %4d %s",
				     getName(), getSerial(), getPath());
	}

	public String toShortString() {
		String	name = getName();
		if (name == null)
			name = "Altus Metrum";
		return String.format("%s %d %s",
				     name, getSerial(), getPath());

	}

	public boolean isAltusMetrum() {
		if (getVendor() != vendor_altusmetrum)
			return false;
		if (getProduct() < product_altusmetrum_min)
			return false;
		if (getProduct() > product_altusmetrum_max)
			return false;
		return true;
	}

	public boolean matchProduct(int want_product) {

		if (!isAltusMetrum())
			return false;

		if (want_product == product_any)
			return true;

		if (want_product == product_basestation)
			return matchProduct(product_teledongle) || matchProduct(product_teleterra);

		int have_product = getProduct();

		if (have_product == product_altusmetrum)	/* old devices match any request */
			return true;

		if (want_product == have_product)
			return true;

		return false;
	}

	static AltosDevice[] list(int product) {
		if (!load_library())
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
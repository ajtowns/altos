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
import java.io.*;
import java.util.*;
import altosui.AltosDeviceName;
import altosui.AltosDeviceNameLinux;
import altosui.AltosDevice;

public class AltosDeviceLinux extends AltosDevice {

	String load_string(File file) {
		try {
			FileInputStream	in = new FileInputStream(file);
			String result = "";
			int c;
			try {
				while ((c = in.read()) != -1) {
					if (c == '\n')
						break;
					result = result + (char) c;
				}
				return result;
			} catch (IOException ee) {
				return "";
			}
		} catch (FileNotFoundException ee) {
			return "";
		}
	}
	String load_string(File dir, String name) {
		return load_string(new File(dir, name));
	}

	int load_hex(File file) {
		try {
			return Integer.parseInt(load_string(file).trim(), 16);
		} catch (NumberFormatException ee) {
			return -1;
		}
	}

	int load_hex(File dir, String name) {
		return load_hex(new File(dir, name));
	}

	int load_dec(File file) {
		try {
			return Integer.parseInt(load_string(file).trim());
		} catch (NumberFormatException ee) {
			return -1;
		}
	}

	int load_dec(File dir, String name) {
		return load_dec(new File(dir, name));
	}

	String usb_tty(File sys_dir) {
		String base = sys_dir.getName();
		int num_configs = load_hex(sys_dir, "bNumConfigurations");
		int num_inters = load_hex(sys_dir, "bNumInterfaces");
		for (int config = 1; config <= num_configs; config++) {
			for (int inter = 0; inter < num_inters; inter++) {
				String endpoint_base = String.format("%s:%d.%d",
								     base, config, inter);
				File endpoint_full = new File(sys_dir, endpoint_base);

				File[] namelist;

				/* Check for tty:ttyACMx style names */
				class tty_colon_filter implements FilenameFilter {
					public boolean accept(File dir, String name) {
						return name.startsWith("tty:");
					}
				}
				namelist = endpoint_full.listFiles(new tty_colon_filter());
				if (namelist != null && namelist.length > 0)
					return new File ("/dev", namelist[0].getName().substring(4)).getPath();

				/* Check for tty/ttyACMx style names */
				class tty_filter implements FilenameFilter {
					public boolean accept(File dir, String name) {
						return name.startsWith("tty");
					}
				}
				File tty_dir = new File(endpoint_full, "tty");
				namelist = tty_dir.listFiles(new tty_filter());
				if (namelist != null && namelist.length > 0)
					return new File ("/dev", namelist[0].getName()).getPath();
			}
		}
		return null;
	}

	public AltosDeviceLinux (File sys) {
		sys = sys;
		manufacturer = load_string(sys, "manufacturer");
		product = load_string(sys, "product");
		serial = load_dec(sys, "serial");
		idProduct = load_hex(sys, "idProduct");
		idVendor = load_hex(sys, "idVendor");
		tty = usb_tty(sys);
	}

	public String toString() {
		return manufacturer + " " + product + " " + serial + " " + idProduct + " " + idVendor + " " + tty;
	}
	static public AltosDeviceLinux[] list() {
		LinkedList<AltosDeviceLinux> devices = new LinkedList<AltosDeviceLinux>();

		class dev_filter implements FilenameFilter{
			public boolean accept(File dir, String name) {
				for (int i = 0; i < name.length(); i++) {
					char c = name.charAt(i);
					if (Character.isDigit(c))
						continue;
					if (c == '-')
						continue;
					if (c == '.' && i != 1)
						continue;
					return false;
				}
				return true;
			}
		}

		File usb_devices = new File("/sys/bus/usb/devices");
		File[] devs = usb_devices.listFiles(new dev_filter());
		if (devs != null) {
			for (int e = 0; e < devs.length; e++) {
				AltosDeviceLinux	dev = new AltosDeviceLinux(devs[e]);
				if (dev.idVendor == 0xfffe && dev.tty != null) {
					devices.add(dev);
				}
			}
		}
		AltosDeviceLinux[] foo = new AltosDeviceLinux[devices.size()];
		for (int e = 0; e < devices.size(); e++) {
			foo[e] = devices.get(e);
			System.out.println("Device " + foo[e]);
		}
		return foo;
	}

	static public AltosDeviceLinux[] list(String model) {
		AltosDeviceLinux[] devices = list();
		LinkedList<AltosDeviceLinux> subset = new LinkedList<AltosDeviceLinux>();
		for (int i = 0; i < devices.length; i++) {
			if (devices[i].product.startsWith(model))
				subset.add(devices[i]);
		}
		return (AltosDeviceLinux[]) subset.toArray();
	}
}

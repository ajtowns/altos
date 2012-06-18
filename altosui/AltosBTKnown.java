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
import java.util.prefs.*;

public class AltosBTKnown implements Iterable<AltosBTDevice> {
	LinkedList<AltosBTDevice>	devices = new LinkedList<AltosBTDevice>();
	Preferences			bt_pref = AltosUIPreferences.bt_devices();

	private String get_address(String name) {
		return bt_pref.get(name, "");
	}

	private void set_address(String name, String addr) {
		bt_pref.put(name, addr);
		System.out.printf("saving known %s %s\n", name, addr);
	}

	private void remove(String name) {
		bt_pref.remove(name);
	}

	private void load() {
		try {
			String[] names = bt_pref.keys();
			for (int i = 0; i < names.length; i++) {
				String	name = names[i];
				String	addr = get_address(name);
				System.out.printf("Known device %s %s\n", name, addr);
				devices.add(new AltosBTDevice(name, addr));
			}
		} catch (BackingStoreException be) {
		} catch (IllegalStateException ie) {
		}
	}

	public Iterator<AltosBTDevice> iterator() {
		return devices.iterator();
	}

	private void flush() {
		AltosUIPreferences.flush_preferences();
	}

	public void set(Iterable<AltosBTDevice> new_devices) {
		for (AltosBTDevice old : devices) {
			boolean found = false;
			for (AltosBTDevice new_device : new_devices) {
				if (new_device.equals(old)) {
					found = true;
					break;
				}
			}
			if (!found)
				remove(old.getName());
		}
		devices = new LinkedList<AltosBTDevice>();
		for (AltosBTDevice new_device : new_devices) {
			devices.add(new_device);
			set_address(new_device.getName(), new_device.getAddr());
		}
		flush();
	}

	public List<AltosDevice> list(int product) {
		LinkedList<AltosDevice>	list = new LinkedList<AltosDevice>();
		for (AltosBTDevice device : devices) {
			if (device.matchProduct(product))
				list.add(device);
		}
		return list;
	}

	public AltosBTKnown() {
		devices = new LinkedList<AltosBTDevice>();
		bt_pref = AltosUIPreferences.bt_devices();
		load();
	}

	static AltosBTKnown	known;

	static public AltosBTKnown bt_known() {
		if (known == null)
			known = new AltosBTKnown();
		return known;
	}
}

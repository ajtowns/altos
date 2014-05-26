/*
 * Copyright © 2012 Mike Beattie <mike@ethernal.org>
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

import java.io.File;
import java.util.prefs.*;
import org.altusmetrum.altoslib_4.*;
import javax.swing.filechooser.FileSystemView;

public class AltosUIPreferencesBackend implements AltosPreferencesBackend {

	private Preferences _preferences = null;

	public AltosUIPreferencesBackend() {
		_preferences = Preferences.userRoot().node("/org/altusmetrum/altosui");
	}

	public AltosUIPreferencesBackend(Preferences in_preferences) {
		_preferences = in_preferences;
	}

	public String  getString(String key, String def) {
		return _preferences.get(key, def);
	}
	public void    putString(String key, String value) {
		_preferences.put(key, value);
	}

	public int     getInt(String key, int def) {
		return _preferences.getInt(key, def);
	}
	public void    putInt(String key, int value) {
		_preferences.putInt(key, value);
	}

	public double  getDouble(String key, double def) {
		return _preferences.getDouble(key, def);
	}
	public void    putDouble(String key, double value) {
		_preferences.putDouble(key, value);
	}

	public boolean getBoolean(String key, boolean def) {
		return _preferences.getBoolean(key, def);
	}
	public void    putBoolean(String key, boolean value) {
		_preferences.putBoolean(key, value);
	}

	public boolean nodeExists(String key) {
		try {
			return _preferences.nodeExists(key);
		} catch (BackingStoreException be) {
			return false;
		}
	}

	public AltosPreferencesBackend node(String key) {
		return new AltosUIPreferencesBackend(_preferences.node(key));
	}

	public String[] keys() {
		try {
			return _preferences.keys();
		} catch (BackingStoreException be) {
			return null;
		}
	}

	public void remove(String key) {
		_preferences.remove(key);
	}

	public void    flush() {
		try {
			_preferences.flush();
		} catch (BackingStoreException ee) {
			System.err.printf("Cannot save preferences\n");
		}
	}

	public File homeDirectory() {
		/* Use the file system view default directory */
		return FileSystemView.getFileSystemView().getDefaultDirectory();
	}
}

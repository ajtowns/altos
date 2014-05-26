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

package org.altusmetrum.AltosDroid;

import java.io.File;
import java.util.Map;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Environment;

import org.altusmetrum.altoslib_4.*;

public class AltosDroidPreferences implements AltosPreferencesBackend {
	public final static String        NAME    = "org.altusmetrum.AltosDroid";
	private Context                   context = null;
	private SharedPreferences         prefs   = null;
	private SharedPreferences.Editor  editor  = null;

	public AltosDroidPreferences(Context in_context) {
		this(in_context, NAME);
	}

	public AltosDroidPreferences(Context in_context, String in_prefs) {
		context = in_context;
		prefs   = context.getSharedPreferences(in_prefs, 0);
		editor  = prefs.edit();
	}

	public String[] keys() {
		Map<String, ?> all = prefs.getAll();
		return (String[])all.keySet().toArray();
	}

	public AltosPreferencesBackend node(String key) {
		return new AltosDroidPreferences(context, key);
	}

	public boolean nodeExists(String key) {
		return prefs.contains(key);
	}

	public boolean getBoolean(String key, boolean def) {
		return prefs.getBoolean(key, def);
	}

	public double getDouble(String key, double def) {
		Float f = Float.valueOf(prefs.getFloat(key, (float)def));
		return f.doubleValue();
	}

	public int getInt(String key, int def) {
		return prefs.getInt(key, def);
	}

	public String getString(String key, String def) {
		return prefs.getString(key, def);
	}

	public void putBoolean(String key, boolean value) {
		editor.putBoolean(key, value);
	}

	public void putDouble(String key, double value) {
		editor.putFloat(key, (float)value);
	}

	public void putInt(String key, int value) {
		editor.putInt(key, value);
	}

	public void putString(String key, String value) {
		editor.putString(key, value);
	}

	public void remove(String key) {
		editor.remove(key);
	}

	public void flush() {
		editor.apply();
	}

	public File homeDirectory() {
		return Environment.getExternalStorageDirectory();
	}
}

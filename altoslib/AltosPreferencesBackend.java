/*
 * Copyright © 2010 Mike Beattie <mike@ethernal.org>
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

package org.altusmetrum.altoslib_4;

import java.io.File;

public interface AltosPreferencesBackend {

	public String  getString(String key, String def);
	public void    putString(String key, String value);

	public int     getInt(String key, int def);
	public void    putInt(String key, int value);

	public double  getDouble(String key, double def);
	public void    putDouble(String key, double value);

	public boolean getBoolean(String key, boolean def);
	public void    putBoolean(String key, boolean value);

	public boolean nodeExists(String key);
	public AltosPreferencesBackend node(String key);

	public String[] keys();
	public void    remove(String key);

	public void    flush();

	public File homeDirectory();
}

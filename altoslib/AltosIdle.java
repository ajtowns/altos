/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

import java.io.*;
import java.util.*;
import java.text.*;
import java.util.concurrent.*;

public abstract class AltosIdle {
	AltosLink	link;
	AltosConfigData	config_data;

	public void printf(String format, Object ... arguments) {
		link.printf(format, arguments);
	}

	public abstract void update_state(AltosState state) throws InterruptedException, TimeoutException;

	public AltosIdle(AltosLink link, AltosConfigData config_data) {
		this.link = link;
		this.config_data = config_data;
	}
}

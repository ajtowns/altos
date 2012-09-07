/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

package org.altusmetrum.AltosLib;

import java.util.concurrent.TimeoutException;

class AltosMs5607Query extends AltosMs5607 {
	public AltosMs5607Query (AltosLink link) throws InterruptedException, TimeoutException {
		link.printf("v\nB\n");
		for (;;) {
			String line = link.get_reply_no_dialog(5000);
			if (line == null) {
				throw new TimeoutException();
			}
			String[] items = line.split("\\s+");
			if (line.startsWith("Pressure:")) {
				if (items.length >= 2)
					raw_pres = Integer.parseInt(items[1]);
			} else if (line.startsWith("Temperature:")) {
				if (items.length >= 2)
					raw_temp = Integer.parseInt(items[1]);
			} else if (line.startsWith("ms5607 reserved:")) {
				if (items.length >= 3)
					reserved = Integer.parseInt(items[2]);
			} else if (line.startsWith("ms5607 sens:")) {
				if (items.length >= 3)
					sens = Integer.parseInt(items[2]);
			} else if (line.startsWith("ms5607 off:")) {
				if (items.length >= 3)
					off = Integer.parseInt(items[2]);
			} else if (line.startsWith("ms5607 tcs:")) {
				if (items.length >= 3)
					tcs = Integer.parseInt(items[2]);
			} else if (line.startsWith("ms5607 tco:")) {
				if (items.length >= 3)
					tco = Integer.parseInt(items[2]);
			} else if (line.startsWith("ms5607 tref:")) {
				if (items.length >= 3)
					tref = Integer.parseInt(items[2]);
			} else if (line.startsWith("ms5607 tempsens:")) {
				if (items.length >= 3)
					tempsens = Integer.parseInt(items[2]);
			} else if (line.startsWith("ms5607 crc:")) {
				if (items.length >= 3)
					crc = Integer.parseInt(items[2]);
			} else if (line.startsWith("Altitude"))
				break;
		}
		convert();
	}
}


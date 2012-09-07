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

package org.altusmetrum.AltosLib;

import java.io.*;
import java.util.concurrent.*;

public class AltosIgnite {
	AltosLink	link;
	boolean		remote;
	boolean		link_started;

	public final static int	None = 0;
	public final static int	Apogee = 1;
	public final static int	Main = 2;

	public final static int	Unknown = 0;
	public final static int	Ready = 1;
	public final static int	Active = 2;
	public final static int	Open = 3;

	private void start_link() throws InterruptedException, TimeoutException {
		link_started = true;
		if (remote)
			link.start_remote();
	}

	private void stop_link() throws InterruptedException {
		if (!link_started)
			return;
		link_started = false;
		if (link == null)
			return;
		if (remote)
			link.stop_remote();
	}

	class string_ref {
		String	value;

		public String get() {
			return value;
		}
		public void set(String i) {
			value = i;
		}
		public string_ref() {
			value = null;
		}
	}

	/*
	private boolean get_string(String line, String label, string_ref s) {
		if (line.startsWith(label)) {
			String	quoted = line.substring(label.length()).trim();

			if (quoted.startsWith("\""))
				quoted = quoted.substring(1);
			if (quoted.endsWith("\""))
				quoted = quoted.substring(0,quoted.length()-1);
			s.set(quoted);
			return true;
		} else {
			return false;
		}
	}
	*/

	private int status(String status_name) {
		if (status_name.equals("unknown"))
			return Unknown;
		if (status_name.equals("ready"))
			return Ready;
		if (status_name.equals("active"))
			return Active;
		if (status_name.equals("open"))
			return Open;
		return Unknown;
	}

	public int status(int igniter) throws InterruptedException, TimeoutException {
		int status = Unknown;
		if (link == null)
			return status;
		//string_ref status_name = new string_ref();
		try {
			start_link();
			link.printf("t\n");
			for (;;) {
				String line = link.get_reply(5000);
				if (line == null)
					throw new TimeoutException();
				String[] items = line.split("\\s+");

				if (items.length < 4)
					continue;

				if (!items[0].equals("Igniter:"))
					continue;

				if (!items[2].equals("Status:"))
					continue;

				if (items[1].equals("drogue")) {
					if (igniter == Apogee)
						status = status(items[3]);
				} else if (items[1].equals("main")) {
					if (igniter == Main)
						status = status(items[3]);
					break;
				}
			}
		} finally {
			stop_link();
		}
		return status;
	}

	public static String status_string(int status) {
		switch (status) {
		case Unknown: return "Unknown";
		case Ready: return "Ready";
		case Active: return "Active";
		case Open: return "Open";
		default: return "Unknown";
		}
	}

	public void fire(int igniter) {
		if (link == null)
			return;
		try {
			start_link();
			switch (igniter) {
			case Main:
				link.printf("i DoIt main\n");
				break;
			case Apogee:
				link.printf("i DoIt drogue\n");
				break;
			}
		} catch (InterruptedException ie) {
		} catch (TimeoutException te) {
		} finally {
			try {
				stop_link();
			} catch (InterruptedException ie) {
			}
		}
	}

	public void close() {
		try {
			stop_link();
		} catch (InterruptedException ie) {
		}
		link.close();
		link = null;
	}

	public AltosIgnite(AltosLink in_link, boolean in_remote)
		throws FileNotFoundException, TimeoutException, InterruptedException {

		link = in_link;
		remote = in_remote;
	}
}
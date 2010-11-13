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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.table.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.util.concurrent.LinkedBlockingQueue;

public class AltosEepromRecord {
	public int	cmd;
	public int	tick;
	public int	a;
	public int	b;
	public String	data;
	public boolean	tick_valid;

	public AltosEepromRecord (String line) {
		tick_valid = false;
		tick = 0;
		a = 0;
		b = 0;
		data = null;
		if (line == null) {
			cmd = Altos.AO_LOG_INVALID;
			data = "";
		} else {
			try {
				String[] tokens = line.split("\\s+");

				if (tokens[0].length() == 1) {
					if (tokens.length != 4) {
						cmd = Altos.AO_LOG_INVALID;
						data = line;
					} else {
						cmd = tokens[0].codePointAt(0);
						tick = Integer.parseInt(tokens[1],16);
						tick_valid = true;
						a = Integer.parseInt(tokens[2],16);
						b = Integer.parseInt(tokens[3],16);
					}
				} else if (tokens[0].equals("Config") && tokens[1].equals("version:")) {
					cmd = Altos.AO_LOG_CONFIG_VERSION;
					data = tokens[2];
				} else if (tokens[0].equals("Main") && tokens[1].equals("deploy:")) {
					cmd = Altos.AO_LOG_MAIN_DEPLOY;
					a = Integer.parseInt(tokens[2]);
				} else if (tokens[0].equals("Apogee") && tokens[1].equals("delay:")) {
					cmd = Altos.AO_LOG_APOGEE_DELAY;
					a = Integer.parseInt(tokens[2]);
				} else if (tokens[0].equals("Radio") && tokens[1].equals("channel:")) {
					cmd = Altos.AO_LOG_RADIO_CHANNEL;
					a = Integer.parseInt(tokens[2]);
				} else if (tokens[0].equals("Callsign:")) {
					cmd = Altos.AO_LOG_CALLSIGN;
					data = tokens[1].replaceAll("\"","");
				} else if (tokens[0].equals("Accel") && tokens[1].equals("cal")) {
					cmd = Altos.AO_LOG_ACCEL_CAL;
					a = Integer.parseInt(tokens[3]);
					b = Integer.parseInt(tokens[5]);
				} else if (tokens[0].equals("Radio") && tokens[1].equals("cal:")) {
					cmd = Altos.AO_LOG_RADIO_CAL;
					a = Integer.parseInt(tokens[2]);
				} else if (tokens[0].equals("manufacturer")) {
					cmd = Altos.AO_LOG_MANUFACTURER;
					data = tokens[1];
				} else if (tokens[0].equals("product")) {
					cmd = Altos.AO_LOG_PRODUCT;
					data = tokens[1];
				} else if (tokens[0].equals("serial-number")) {
					cmd = Altos.AO_LOG_SERIAL_NUMBER;
					a = Integer.parseInt(tokens[1]);
				} else if (tokens[0].equals("software-version")) {
					cmd = Altos.AO_LOG_SOFTWARE_VERSION;
					data = tokens[1];
				} else {
					cmd = Altos.AO_LOG_INVALID;
					data = line;
				}
			} catch (NumberFormatException ne) {
				cmd = Altos.AO_LOG_INVALID;
				data = line;
			}
		}
	}

	public AltosEepromRecord(int in_cmd, int in_tick, int in_a, int in_b) {
		tick_valid = true;
		cmd = in_cmd;
		tick = in_tick;
		a = in_a;
		b = in_b;
	}
}

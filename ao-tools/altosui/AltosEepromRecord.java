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

import altosui.AltosSerial;
import altosui.AltosSerialMonitor;
import altosui.AltosRecord;
import altosui.AltosTelemetry;
import altosui.AltosState;
import altosui.AltosDeviceDialog;
import altosui.AltosPreferences;
import altosui.AltosLog;
import altosui.AltosVoice;
import altosui.AltosEepromMonitor;

public class AltosEepromRecord {
	public int	cmd;
	public int	tick;
	public int	a;
	public int	b;

	public AltosEepromRecord (String line) throws ParseException {
		if (line == null) {
			cmd = Altos.AO_LOG_INVALID;
		} else {
			String[] tokens = line.split("\\s+");

			if (tokens[0].equals("serial-number")) {
				cmd = Altos.AO_LOG_SERIAL_NUMBER;
				tick = 0;
				a = Integer.parseInt(tokens[1]);
				b = 0;
			} else {
				if (tokens.length != 4)
					throw new ParseException(line, 0);
				cmd = tokens[0].codePointAt(0);
				tick = Integer.parseInt(tokens[1]);
				a = Integer.parseInt(tokens[2]);
				b = Integer.parseInt(tokens[3]);
			}
		}
	}

}

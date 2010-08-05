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

import altosui.AltosRecord;
import altosui.AltosState;
import altosui.AltosDeviceDialog;
import altosui.AltosPreferences;
import altosui.AltosLog;
import altosui.AltosVoice;
import altosui.AltosEepromMonitor;

public class AltosEepromReader {

	static final int	seen_flight = 1;
	static final int	seen_sensor = 2;
	static final int	seen_temp_volt = 4;
	static final int	seen_deploy = 8;

	static final int	seen_basic = seen_flight|seen_sensor|seen_temp_volt|seen_deploy;

	static final int	seen_gps_time = 16;

	AltosRecord		state;
	AltosEepromRecord	record;

	int			seen;

	int			tick;

	boolean			done;

	FileInputStream	input;

	public AltosRecord read() throws IOException, ParseException {
		for (;;) {
			if (record == null) {
				record = new AltosEepromRecord(AltosRecord.gets(input));
				if (record == null) {
					if (done)
						return null;
					return state;
				}

				/* eeprom only records low 16 bits of tick count */
				int tick = record.tick | (state.tick & ~0xffff);

				if (tick < state.tick) {
					if (state.tick - tick > 0x8000)
						tick += 0x10000;
					else
						tick = state.tick;
				}

				/* Accumulate data in the state record while
				 * the time stamp is not increasing
				 */

				if ((seen & seen_basic) == seen_basic && tick > state.tick)
					return new AltosRecord(state);
			}

			state.tick = tick;
			switch (record.cmd) {
			case Altos.AO_LOG_FLIGHT:
				state.ground_accel = record.a;
				state.flight = record.b;
				break;
			case Altos.AO_LOG_SENSOR:
				state.accel = record.a;
				state.pres = record.b;
				break;
			case Altos.AO_LOG_TEMP_VOLT:
				state.temp = record.a;
				state.batt = record.b;
				break;
			case Altos.AO_LOG_DEPLOY:
				state.drogue = record.a;
				state.main = record.b;
				break;
			case Altos.AO_LOG_GPS_TIME:
				break;
			}
			record = null;
		}
	}

	public AltosEepromReader (FileInputStream in_input) {
		state = new AltosRecord();
		input = in_input;
		seen = 0;
		done = false;
	}
}

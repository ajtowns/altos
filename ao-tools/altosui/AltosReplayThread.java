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

import altosui.Altos;
import altosui.AltosSerial;
import altosui.AltosSerialMonitor;
import altosui.AltosRecord;
import altosui.AltosTelemetry;
import altosui.AltosState;
import altosui.AltosDeviceDialog;
import altosui.AltosPreferences;
import altosui.AltosLog;
import altosui.AltosVoice;
import altosui.AltosFlightInfoTableModel;
import altosui.AltosChannelMenu;
import altosui.AltosFlashUI;
import altosui.AltosLogfileChooser;
import altosui.AltosCSVUI;
import altosui.AltosLine;
import altosui.AltosStatusTable;
import altosui.AltosInfoTable;
import altosui.AltosDisplayThread;

/*
 * Open an existing telemetry file and replay it in realtime
 */

public class AltosReplayThread extends AltosDisplayThread {
	Iterator<AltosRecord>	iterator;
	String			name;

	public AltosRecord read() {
		if (iterator.hasNext())
			return iterator.next();
		return null;
	}

	public void close (boolean interrupted) {
		if (!interrupted)
			report();
	}

	void update(AltosState state) throws InterruptedException {
		/* Make it run in realtime after the rocket leaves the pad */
		if (state.state > Altos.ao_flight_pad)
			Thread.sleep((int) (Math.min(state.time_change,10) * 1000));
	}

	public AltosReplayThread(Frame parent, Iterator<AltosRecord> in_iterator,
				 String in_name, AltosVoice voice,
				 AltosStatusTable status, AltosInfoTable info) {
		super(parent, voice, status, info);
		iterator = in_iterator;
		name = in_name;
	}
}

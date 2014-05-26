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
/*
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
import org.altusmetrum.altoslib_4.*;

public class AltosFlightStatusTableModel extends AbstractTableModel {
	private String[] columnNames = {
		String.format("Height (%s)", AltosConvert.show_distance_units()),
		"State",
		"RSSI (dBm)",
		String.format("Speed (%s)", AltosConvert.show_speed_unit())
	};
	private Object[] data = { 0, "idle", 0, 0 };

	public int getColumnCount() { return columnNames.length; }
	public int getRowCount() { return 2; }
	public Object getValueAt(int row, int col) {
		if (row == 0)
			return columnNames[col];
		return data[col];
	}

	public void setValueAt(Object value, int col) {
		data[col] = value;
		fireTableCellUpdated(1, col);
	}

	public void setValueAt(Object value, int row, int col) {
		setValueAt(value, col);
	}

	public void set(AltosState state) {
		setValueAt(String.format("%1.0f", AltosConvert.distance(state.height), 0);
		setValueAt(state.data.state(), 1);
		setValueAt(state.data.rssi, 2);
		double speed = state.baro_speed;
		if (state.ascent)
			speed = state.speed;
		setValueAt(String.format("%1.0f", AltosConvert.speed(speed)), 3);
	}
}
*/
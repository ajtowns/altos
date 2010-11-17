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

public class AltosFlightInfoTableModel extends AbstractTableModel {
	private String[] columnNames = {"Field", "Value"};

	class InfoLine {
		String	name;
		String	value;

		public InfoLine(String n, String v) {
			name = n;
			value = v;
		}
	}

	private ArrayList<InfoLine> rows = new ArrayList<InfoLine>();

	public int getColumnCount() { return columnNames.length; }
	public String getColumnName(int col) { return columnNames[col]; }

	public int getRowCount() { return 17; }

	int	current_row = 0;
	int	prev_num_rows = 0;

	public Object getValueAt(int row, int col) {
		if (row >= rows.size())
			return "";
		if (col == 0)
			return rows.get(row).name;
		else
			return rows.get(row).value;
	}

	public void resetRow() {
		current_row = 0;
	}
	public void addRow(String name, String value) {
		if (current_row >= rows.size())
			rows.add(current_row, new InfoLine(name, value));
		else
			rows.set(current_row, new InfoLine(name, value));
		current_row++;
	}
	public void finish() {
		if (current_row > prev_num_rows)
			fireTableRowsInserted(prev_num_rows, current_row - 1);
		while (rows.size() > current_row)
			rows.remove(rows.size() - 1);
		prev_num_rows = current_row;
		fireTableDataChanged();
	}
}

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

package org.altusmetrum.altosuilib_2;

import javax.swing.table.*;

public class AltosFlightInfoTableModel extends AbstractTableModel {
	final static private String[] columnNames = {"Field", "Value"};

	int	rows;
	int	cols;
	private String[][] data;

	public int getColumnCount() { return cols; }
	public int getRowCount() { return rows; }
	public String getColumnName(int col) { return columnNames[col & 1]; }

	public Object getValueAt(int row, int col) {
		if (row >= rows || col >= cols)
			return "";
		return data[row][col];
	}

	int[]	current_row;

	public void reset() {
		for (int i = 0; i < cols / 2; i++)
			current_row[i] = 0;
	}

	public void clear() {
		reset();
		for (int c = 0; c < cols; c++)
			for (int r = 0; r < rows; r++)
				data[r][c] = "";
		fireTableDataChanged();
	}

	public void addRow(int col, String name, String value) {
		if (current_row[col] < rows) {
			data[current_row[col]][col * 2] = name;
			data[current_row[col]][col * 2 + 1] = value;
		}
		current_row[col]++;
	}

	public void finish() {
		for (int c = 0; c < cols / 2; c++)
			while (current_row[c] < rows)
				addRow(c, "", "");
		fireTableDataChanged();
	}

	public AltosFlightInfoTableModel (int in_rows, int in_cols) {
		rows = in_rows;
		cols = in_cols * 2;
		data = new String[rows][cols];
		current_row = new int[in_cols];
	}
}

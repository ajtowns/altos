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

import altosui.AltosFlightStatusTableModel;
import altosui.AltosFlightInfoTableModel;

public class AltosStatusTable extends JTable {
	private AltosFlightStatusTableModel flightStatusModel;

	private Font statusFont = new Font("SansSerif", Font.BOLD, 24);

	public AltosStatusTable() {
		super((TableModel) new AltosFlightStatusTableModel());
		flightStatusModel = (AltosFlightStatusTableModel) getModel();

		setFont(statusFont);

		TableColumnModel tcm = getColumnModel();

		for (int i = 0; i < flightStatusModel.getColumnCount(); i++) {
			DefaultTableCellRenderer       r = new DefaultTableCellRenderer();
			r.setFont(statusFont);
			r.setHorizontalAlignment(SwingConstants.CENTER);
			tcm.getColumn(i).setCellRenderer(r);
		}

		setRowHeight(rowHeight());
		setShowGrid(false);
	}

	public int rowHeight() {
		FontMetrics	statusMetrics = getFontMetrics(statusFont);
		return (statusMetrics.getHeight() + statusMetrics.getLeading()) * 15 / 10;
	}

	public int height() {
		return rowHeight * 4;
	}

	public void set(AltosState state) {
		flightStatusModel.set(state);
	}
}

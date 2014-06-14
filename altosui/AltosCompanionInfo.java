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
import javax.swing.*;
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

public class AltosCompanionInfo extends JTable implements AltosFlightDisplay {
	private AltosFlightInfoTableModel model;

	static final int info_columns = 2;
	static final int info_rows = 17;

	int desired_row_height() {
		FontMetrics	infoValueMetrics = getFontMetrics(Altos.table_value_font);
		return (infoValueMetrics.getHeight() + infoValueMetrics.getLeading()) * 18 / 10;
	}

	public void font_size_changed(int font_size) {
		setFont(Altos.table_value_font);
		setRowHeight(desired_row_height());
		doLayout();
	}

	public void units_changed(boolean imperial_units) {
	}

	public AltosCompanionInfo() {
		super(new AltosFlightInfoTableModel(info_rows, info_columns));
		model = (AltosFlightInfoTableModel) getModel();
		setAutoResizeMode(AUTO_RESIZE_ALL_COLUMNS);
		setShowGrid(true);
		font_size_changed(AltosUIPreferences.font_size());
	}

	public Dimension getPreferredScrollableViewportSize() {
		return getPreferredSize();
	}

	public void reset() {
		model.reset();
	}

	void info_add_row(int col, String name, String value) {
		model.addRow(col, name, value);
	}

	void info_add_row(int col, String name, String format, Object... parameters) {
		info_add_row (col, name, String.format(format, parameters));
	}

	void info_finish() {
		model.finish();
	}

	public void clear() {
		model.clear();
	}

	AltosCompanion	companion;

	public String board_name() {
		if (companion == null)
			return "None";
		switch (companion.board_id) {
		case AltosCompanion.board_id_telescience:
			return "TeleScience";
		default:
			return String.format("%02x\n", companion.board_id);
		}
	}

	public String getName() { return "Companion"; }

	public void show(AltosState state, AltosListenerState listener_state) {
		if (state == null)
			return;
		if (state.companion != null)
			companion = state.companion;
		reset();
		info_add_row(0, "Companion board", "%s", board_name());
		if (companion != null) {
			info_add_row(0, "Last Data", "%5d", companion.tick);
			info_add_row(0, "Update period", "%5.2f s",
				     companion.update_period / 100.0);
			info_add_row(0, "Channels", "%3d", companion.channels);

			for (int i = 0; i < companion.channels; i++)
				info_add_row(1, String.format("Channel %2d", i),
					     "%6d", companion.companion_data[i]);
		}
		info_finish();
	}
}

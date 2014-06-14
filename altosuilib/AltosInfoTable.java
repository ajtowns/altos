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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.table.*;
import org.altusmetrum.altoslib_4.*;

public class AltosInfoTable extends JTable implements AltosFlightDisplay, HierarchyListener {
	private AltosFlightInfoTableModel model;

	static final int info_columns = 3;
	static final int info_rows = 17;

	private AltosState		last_state;
	private AltosListenerState	last_listener_state;

	int desired_row_height() {
		FontMetrics	infoValueMetrics = getFontMetrics(AltosUILib.table_value_font);
		return (infoValueMetrics.getHeight() + infoValueMetrics.getLeading()) * 18 / 10;
	}

	int text_width(String t) {
		FontMetrics	infoValueMetrics = getFontMetrics(AltosUILib.table_value_font);

		return infoValueMetrics.stringWidth(t);
	}

	void set_layout() {
		setRowHeight(desired_row_height());
		for (int i = 0; i < info_columns * 2; i++)
		{
			TableColumn column = getColumnModel().getColumn(i);

			if ((i & 1) == 0)
				column.setPreferredWidth(text_width(" Satellites Visible"));
			else
				column.setPreferredWidth(text_width("W 179°59.99999' "));
		}
	}

	public AltosInfoTable() {
		super(new AltosFlightInfoTableModel(info_rows, info_columns));
		model = (AltosFlightInfoTableModel) getModel();
		setFont(AltosUILib.table_value_font);
		addHierarchyListener(this);
		setAutoResizeMode(AUTO_RESIZE_ALL_COLUMNS);
		setShowGrid(true);
		set_layout();
		doLayout();
	}

	public void font_size_changed(int font_size) {
		setFont(AltosUILib.table_value_font);
		set_layout();
		doLayout();
	}

	public void units_changed(boolean imperial_units) {
	}

	public void hierarchyChanged(HierarchyEvent e) {
		if (last_state != null && isShowing()) {
			AltosState		state = last_state;
			AltosListenerState	listener_state = last_listener_state;

			last_state = null;
			last_listener_state = null;
			show(state, listener_state);
		}
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

	void info_add_deg(int col, String name, double v, int pos, int neg) {
		int	c = pos;
		if (v < 0) {
			c = neg;
			v = -v;
		}
		double	deg = Math.floor(v);
		double	min = (v - deg) * 60;

		info_add_row(col, name, String.format("%c %3.0f°%08.5f'", c, deg, min));
	}

	void info_finish() {
		model.finish();
	}

	public void clear() {
		model.clear();
	}

	public String getName() { return "Table"; }

	public void show(AltosState state, AltosListenerState listener_state) {

		if (!isShowing()) {
			last_state = state;
			last_listener_state = listener_state;
			return;
		}

		reset();
		if (state != null) {
			if (state.device_type != AltosLib.MISSING)
				info_add_row(0, "Device", "%s", AltosLib.product_name(state.device_type));
			if (state.altitude() != AltosLib.MISSING)
				info_add_row(0, "Altitude", "%6.0f    m", state.altitude());
			if (state.ground_altitude() != AltosLib.MISSING)
				info_add_row(0, "Pad altitude", "%6.0f    m", state.ground_altitude());
			if (state.height() != AltosLib.MISSING)
				info_add_row(0, "Height", "%6.0f    m", state.height());
			if (state.max_height() != AltosLib.MISSING)
				info_add_row(0, "Max height", "%6.0f    m", state.max_height());
			if (state.acceleration() != AltosLib.MISSING)
				info_add_row(0, "Acceleration", "%8.1f  m/s²", state.acceleration());
			if (state.max_acceleration() != AltosLib.MISSING)
				info_add_row(0, "Max acceleration", "%8.1f  m/s²", state.max_acceleration());
			if (state.speed() != AltosLib.MISSING)
				info_add_row(0, "Speed", "%8.1f  m/s", state.speed());
			if (state.max_speed() != AltosLib.MISSING)
				info_add_row(0, "Max Speed", "%8.1f  m/s", state.max_speed());
			if (state.orient() != AltosLib.MISSING)
				info_add_row(0, "Tilt", "%4.0f °", state.orient());
			if (state.max_orient() != AltosLib.MISSING)
				info_add_row(0, "Max Tilt", "%4.0f °", state.max_orient());
			if (state.temperature != AltosLib.MISSING)
				info_add_row(0, "Temperature", "%9.2f °C", state.temperature);
			if (state.battery_voltage != AltosLib.MISSING)
				info_add_row(0, "Battery", "%9.2f V", state.battery_voltage);
			if (state.apogee_voltage != AltosLib.MISSING)
				info_add_row(0, "Drogue", "%9.2f V", state.apogee_voltage);
			if (state.main_voltage != AltosLib.MISSING)
				info_add_row(0, "Main", "%9.2f V", state.main_voltage);
		}
		if (listener_state != null) {
			info_add_row(0, "CRC Errors", "%6d", listener_state.crc_errors);

			if (listener_state.battery != AltosLib.MISSING)
				info_add_row(0, "Receiver Battery", "%9.2f", listener_state.battery);
		}

		if (state != null) {
			if (state.gps == null || !state.gps.connected) {
				info_add_row(1, "GPS", "not available");
			} else {
				if (state.gps_ready)
					info_add_row(1, "GPS state", "%s", "ready");
				else
					info_add_row(1, "GPS state", "wait (%d)",
						     state.gps_waiting);
				if (state.gps.locked)
					info_add_row(1, "GPS", "   locked");
				else if (state.gps.connected)
					info_add_row(1, "GPS", " unlocked");
				else
					info_add_row(1, "GPS", "  missing");
				info_add_row(1, "Satellites", "%6d", state.gps.nsat);
				if (state.gps.lat != AltosLib.MISSING)
					info_add_deg(1, "Latitude", state.gps.lat, 'N', 'S');
				if (state.gps.lon != AltosLib.MISSING)
					info_add_deg(1, "Longitude", state.gps.lon, 'E', 'W');
				if (state.gps.alt != AltosLib.MISSING)
					info_add_row(1, "GPS altitude", "%8.1f", state.gps.alt);
				if (state.gps_height != AltosLib.MISSING)
					info_add_row(1, "GPS height", "%8.1f", state.gps_height);

				/* The SkyTraq GPS doesn't report these values */
				/*
				  if (false) {
				  info_add_row(1, "GPS ground speed", "%8.1f m/s %3d°",
				  state.gps.ground_speed,
				  state.gps.course);
				  info_add_row(1, "GPS climb rate", "%8.1f m/s",
				  state.gps.climb_rate);
				  info_add_row(1, "GPS error", "%6d m(h)%3d m(v)",
				  state.gps.h_error, state.gps.v_error);
				  }
				*/

				info_add_row(1, "GPS hdop", "%8.1f", state.gps.hdop);

				if (state.npad > 0) {
					if (state.from_pad != null) {
						info_add_row(1, "Distance from pad", "%6d m",
							     (int) (state.from_pad.distance + 0.5));
						info_add_row(1, "Direction from pad", "%6d°",
							     (int) (state.from_pad.bearing + 0.5));
						info_add_row(1, "Elevation from pad", "%6d°",
							     (int) (state.elevation + 0.5));
						info_add_row(1, "Range from pad", "%6d m",
							     (int) (state.range + 0.5));
					} else {
						info_add_row(1, "Distance from pad", "unknown");
						info_add_row(1, "Direction from pad", "unknown");
						info_add_row(1, "Elevation from pad", "unknown");
						info_add_row(1, "Range from pad", "unknown");
					}
					info_add_deg(1, "Pad latitude", state.pad_lat, 'N', 'S');
					info_add_deg(1, "Pad longitude", state.pad_lon, 'E', 'W');
					info_add_row(1, "Pad GPS alt", "%6.0f m", state.pad_alt);
				}
				if (state.gps.year != AltosLib.MISSING)
					info_add_row(1, "GPS date", "%04d-%02d-%02d",
						     state.gps.year,
						     state.gps.month,
						     state.gps.day);
				if (state.gps.hour != AltosLib.MISSING)
					info_add_row(1, "GPS time", "  %02d:%02d:%02d",
						     state.gps.hour,
						     state.gps.minute,
						     state.gps.second);
				//int	nsat_vis = 0;
				int	c;

				if (state.gps.cc_gps_sat == null)
					info_add_row(2, "Satellites Visible", "%4d", 0);
				else {
					info_add_row(2, "Satellites Visible", "%4d", state.gps.cc_gps_sat.length);
					for (c = 0; c < state.gps.cc_gps_sat.length; c++) {
						info_add_row(2, "Satellite id,C/N0",
							     "%4d, %4d",
							     state.gps.cc_gps_sat[c].svid,
							     state.gps.cc_gps_sat[c].c_n0);
					}
				}
			}
		}
		info_finish();
	}
}

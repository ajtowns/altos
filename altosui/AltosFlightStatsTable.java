/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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
import java.util.concurrent.*;

public class AltosFlightStatsTable extends JComponent {
	GridBagLayout	layout;

	class FlightStat {
		JLabel		label;
		JTextField	value;

		public FlightStat(GridBagLayout layout, int y, String label_text, String ... values) {
			GridBagConstraints	c = new GridBagConstraints();
			c.insets = new Insets(Altos.tab_elt_pad, Altos.tab_elt_pad, Altos.tab_elt_pad, Altos.tab_elt_pad);
			c.weighty = 1;

			label = new JLabel(label_text);
			label.setFont(Altos.label_font);
			label.setHorizontalAlignment(SwingConstants.LEFT);
			c.gridx = 0; c.gridy = y;
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.VERTICAL;
			c.weightx = 0;
			layout.setConstraints(label, c);
			add(label);

			for (int j = 0; j < values.length; j++) {
				value = new JTextField(values[j]);
				value.setFont(Altos.value_font);
				value.setHorizontalAlignment(SwingConstants.RIGHT);
				c.gridx = j+1; c.gridy = y;
				c.anchor = GridBagConstraints.EAST;
				c.fill = GridBagConstraints.BOTH;
				c.weightx = 1;
				layout.setConstraints(value, c);
				add(value);
			}
		}

	}

	public AltosFlightStatsTable(AltosFlightStats stats) {
		layout = new GridBagLayout();

		setLayout(layout);
		int y = 0;
		new FlightStat(layout, y++, "Maximum height",
			       String.format("%5.0f m", stats.max_height),
			       String.format("%5.0f ft", stats.max_height * 100 / 2.54 / 12));
		new FlightStat(layout, y++, "Maximum speed",
			       String.format("%5.0f m/s", stats.max_speed),
			       String.format("%5.0f ft/s", stats.max_speed * 100 / 2.54 / 12),
			       String.format("Mach %5.3f", stats.max_speed / 343.0));
		if (stats.max_acceleration != AltosRecord.MISSING) {
			new FlightStat(layout, y++, "Maximum acceleration",
				       String.format("%5.0f m/s²", stats.max_acceleration),
				       String.format("%5.0f ft/s²", stats.max_acceleration * 100 / 2.54 /12),
				       String.format("%5.2f G", stats.max_acceleration / 9.80665));
			new FlightStat(layout, y++, "Average boost acceleration",
				       String.format("%5.0f m/s²", stats.state_accel[Altos.ao_flight_boost]),
				       String.format("%5.0f ft/s²", stats.state_accel[Altos.ao_flight_boost] * 100 / 2.54 /12),
				       String.format("%5.2f G", stats.state_accel[Altos.ao_flight_boost] / 9.80665));
		}
		new FlightStat(layout, y++, "Drogue descent rate",
			       String.format("%5.0f m/s", stats.state_baro_speed[Altos.ao_flight_drogue]),
			       String.format("%5.0f ft/s", stats.state_baro_speed[Altos.ao_flight_drogue] * 100 / 2.54 / 12));
		new FlightStat(layout, y++, "Main descent rate",
			       String.format("%5.0f m/s", stats.state_baro_speed[Altos.ao_flight_main]),
			       String.format("%5.0f ft/s", stats.state_baro_speed[Altos.ao_flight_main] * 100 / 2.54 / 12));
		for (int s = Altos.ao_flight_boost; s <= Altos.ao_flight_main; s++) {
			new FlightStat(layout, y++, String.format("%s time", Altos.state_to_string_capital[s]),
				       String.format("%6.2f s", stats.state_end[s] - stats.state_start[s]));
		}
		new FlightStat(layout, y++, "Flight Time",
			       String.format("%6.2f s", stats.state_end[Altos.ao_flight_main] -
					     stats.state_start[Altos.ao_flight_boost]));
		
	}
	
}
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
import javax.swing.*;
import org.altusmetrum.altoslib_3.*;

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

	static String pos(double p, String pos, String neg) {
		String	h = pos;
		if (p < 0) {
			h = neg;
			p = -p;
		}
		int deg = (int) Math.floor(p);
		double min = (p - Math.floor(p)) * 60.0;
		return String.format("%s %4d° %9.6f'", h, deg, min);
	}

	public AltosFlightStatsTable(AltosFlightStats stats) {
		layout = new GridBagLayout();

		setLayout(layout);
		int y = 0;
		new FlightStat(layout, y++, "Serial", String.format("%d", stats.serial));
		new FlightStat(layout, y++, "Flight", String.format("%d", stats.flight));
		if (stats.year != AltosLib.MISSING && stats.hour != AltosLib.MISSING)
			new FlightStat(layout, y++, "Date/Time",
				       String.format("%04d-%02d-%02d", stats.year, stats.month, stats.day),
				       String.format("%02d:%02d:%02d UTC", stats.hour, stats.minute, stats.second));
		else {
			if (stats.year != AltosLib.MISSING)
				new FlightStat(layout, y++, "Date",
					       String.format("%04d-%02d-%02d", stats.year, stats.month, stats.day));
			if (stats.hour != AltosLib.MISSING)
				new FlightStat(layout, y++, "Time",
					       String.format("%02d:%02d:%02d UTC", stats.hour, stats.minute, stats.second));
		}
		new FlightStat(layout, y++, "Maximum height",
			       String.format("%5.0f m", stats.max_height),
			       String.format("%5.0f ft", AltosConvert.meters_to_feet(stats.max_height)));
		if (stats.max_gps_height != AltosLib.MISSING) {
			new FlightStat(layout, y++, "Maximum GPS height",
				       String.format("%5.0f m", stats.max_gps_height),
				       String.format("%5.0f ft", AltosConvert.meters_to_feet(stats.max_gps_height)));
		}
		new FlightStat(layout, y++, "Maximum speed",
			       String.format("%5.0f m/s", stats.max_speed),
			       String.format("%5.0f mph", AltosConvert.meters_to_mph(stats.max_speed)),
			       String.format("Mach %4.1f", AltosConvert.meters_to_mach(stats.max_speed)));
		if (stats.max_acceleration != AltosLib.MISSING) {
			new FlightStat(layout, y++, "Maximum boost acceleration",
				       String.format("%5.0f m/s²", stats.max_acceleration),
				       String.format("%5.0f ft/s²", AltosConvert.meters_to_feet(stats.max_acceleration)),
				       String.format("%5.0f G", AltosConvert.meters_to_g(stats.max_acceleration)));
			new FlightStat(layout, y++, "Average boost acceleration",
				       String.format("%5.0f m/s²", stats.state_accel[Altos.ao_flight_boost]),
				       String.format("%5.0f ft/s²", AltosConvert.meters_to_feet(stats.state_accel[Altos.ao_flight_boost])),
				       String.format("%5.0f G", AltosConvert.meters_to_g(stats.state_accel[Altos.ao_flight_boost])));
		}
		new FlightStat(layout, y++, "Drogue descent rate",
			       String.format("%5.0f m/s", stats.state_speed[Altos.ao_flight_drogue]),
			       String.format("%5.0f ft/s", AltosConvert.meters_to_feet(stats.state_speed[Altos.ao_flight_drogue])));
		new FlightStat(layout, y++, "Main descent rate",
			       String.format("%5.0f m/s", stats.state_speed[Altos.ao_flight_main]),
			       String.format("%5.0f ft/s", AltosConvert.meters_to_feet(stats.state_speed[Altos.ao_flight_main])));
		new FlightStat(layout, y++, "Ascent time",
			       String.format("%6.1f s %s", stats.state_end[AltosLib.ao_flight_boost] - stats.state_start[AltosLib.ao_flight_boost],
					     AltosLib.state_name(Altos.ao_flight_boost)),
			       String.format("%6.1f s %s", stats.state_end[AltosLib.ao_flight_fast] - stats.state_start[AltosLib.ao_flight_fast],
					     AltosLib.state_name(Altos.ao_flight_fast)),
			       String.format("%6.1f s %s", stats.state_end[AltosLib.ao_flight_coast] - stats.state_start[AltosLib.ao_flight_coast],
					     AltosLib.state_name(Altos.ao_flight_coast)));
		new FlightStat(layout, y++, "Descent time",
			       String.format("%6.1f s %s", stats.state_end[AltosLib.ao_flight_drogue] - stats.state_start[AltosLib.ao_flight_drogue],
					     AltosLib.state_name(Altos.ao_flight_drogue)),
			       String.format("%6.1f s %s", stats.state_end[AltosLib.ao_flight_main] - stats.state_start[AltosLib.ao_flight_main],
					     AltosLib.state_name(Altos.ao_flight_main)));
		new FlightStat(layout, y++, "Flight time",
			       String.format("%6.1f s", stats.state_end[Altos.ao_flight_main] -
					     stats.state_start[Altos.ao_flight_boost]));
		if (stats.has_gps) {
			new FlightStat(layout, y++, "Pad location",
				       pos(stats.pad_lat,"N","S"),
				       pos(stats.pad_lon,"E","W"));
			new FlightStat(layout, y++, "Last reported location",
				       pos(stats.lat,"N","S"),
				       pos(stats.lon,"E","W"));
		}
	}
	
}
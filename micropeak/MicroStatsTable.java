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

package org.altusmetrum.micropeak;

import java.awt.*;
import javax.swing.*;
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

public class MicroStatsTable extends JComponent implements AltosFontListener {
	GridBagLayout	layout;

	class MicroStat {
		JLabel		label;
		JTextField[]	texts;

		public void set_values(String ... values) {
			for (int j = 0; j < values.length; j++) {
				texts[j].setText(values[j]);
			}
		}

		public void set_font() {
			for (int j = 0; j < texts.length; j++)
				texts[j].setFont(AltosUILib.value_font);
			label.setFont(AltosUILib.label_font);
		}

		public MicroStat(GridBagLayout layout, int y, String label_text, String ... values) {
			GridBagConstraints	c = new GridBagConstraints();
			c.insets = new Insets(AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad);
			c.weighty = 1;

			label = new JLabel(label_text);
			label.setFont(AltosUILib.label_font);
			label.setHorizontalAlignment(SwingConstants.LEFT);
			c.gridx = 0; c.gridy = y;
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.VERTICAL;
			c.weightx = 0;
			layout.setConstraints(label, c);
			add(label);

			texts = new JTextField[values.length];
			for (int j = 0; j < values.length; j++) {
				JTextField value = new JTextField(values[j]);
				value.setFont(AltosUILib.value_font);
				value.setHorizontalAlignment(SwingConstants.RIGHT);
				value.setEditable(false);
				texts[j] = value;
				c.gridx = j+1; c.gridy = y;
				c.anchor = GridBagConstraints.EAST;
				c.fill = GridBagConstraints.BOTH;
				c.weightx = 1;
				layout.setConstraints(value, c);
				add(value);
			}
		}
	}

	MicroStat	max_height, max_speed;
	MicroStat	max_accel, avg_accel;
	MicroStat	boost_duration;
	MicroStat	coast_duration;
	MicroStat	descent_speed;
	MicroStat	descent_duration;
	MicroStat	flight_time;
	
	public void setStats(MicroStats stats) {
		max_height.set_values(String.format("%7.1f m", stats.apogee_height),
				      String.format("%7.1f ft", AltosConvert.meters_to_feet(stats.apogee_height)));
		max_speed.set_values(String.format("%7.1f m/s", stats.max_speed),
				     String.format("%7.1f mph", AltosConvert.meters_to_mph(stats.max_speed)),
				     String.format("Mach %7.3f", AltosConvert.meters_to_mach(stats.max_speed)));
		max_accel.set_values(String.format("%7.1f m/s²", stats.max_accel),
				     String.format("%7.1f ft/s²", AltosConvert.meters_to_feet(stats.max_accel)),
				     String.format("%7.3f G", AltosConvert.meters_to_g(stats.max_accel)));
		avg_accel.set_values(String.format("%7.1f m/s²", stats.boost_accel(),
						   String.format("%7.1f ft/s²", AltosConvert.meters_to_feet(stats.boost_accel())),
						   String.format("%7.3f G", AltosConvert.meters_to_g(stats.boost_accel()))));
		boost_duration.set_values(String.format("%6.1f s", stats.boost_duration()));
		coast_duration.set_values(String.format("%6.1f s", stats.coast_duration()));
		descent_speed.set_values(String.format("%7.1f m/s", stats.descent_speed()),
					 String.format("%7.1f ft/s", AltosConvert.meters_to_feet(stats.descent_speed())));
		descent_duration.set_values(String.format("%6.1f s", stats.descent_duration()));
		flight_time.set_values(String.format("%6.1f s", stats.landed_time));
	}

	public void set_font() {
		max_height.set_font();
		max_speed.set_font();
		max_accel.set_font();
		avg_accel.set_font();
		boost_duration.set_font();
		coast_duration.set_font();
		descent_speed.set_font();
		descent_duration.set_font();
		flight_time.set_font();
	}

	public void font_size_changed(int font_size) {
		set_font();
	}

	public MicroStatsTable(MicroStats stats) {
		layout = new GridBagLayout();

		setLayout(layout);
		int y = 0;
		max_height = new MicroStat(layout, y++, "Maximum height",
					   String.format("%7.1f m", stats.apogee_height),
					   String.format("%7.1f ft", AltosConvert.meters_to_feet(stats.apogee_height)));
		max_speed = new MicroStat(layout, y++, "Maximum speed",
					  String.format("%7.1f m/s", stats.max_speed),
					  String.format("%7.1f mph", AltosConvert.meters_to_mph(stats.max_speed)),
					  String.format("Mach %4.1f", AltosConvert.meters_to_mach(stats.max_speed)));
		max_accel = new MicroStat(layout, y++, "Maximum boost acceleration",
					  String.format("%7.1f m/s²", stats.max_accel),
					  String.format("%7.1f ft/s²", AltosConvert.meters_to_feet(stats.max_accel)),
					  String.format("%7.3f G", AltosConvert.meters_to_g(stats.max_accel)));
		avg_accel = new MicroStat(layout, y++, "Average boost acceleration",
					  String.format("%7.1f m/s²", stats.boost_accel(),
							String.format("%7.1f ft/s²", AltosConvert.meters_to_feet(stats.boost_accel())),
							String.format("%7.3f G", AltosConvert.meters_to_g(stats.boost_accel()))));
		boost_duration = new MicroStat(layout, y++, "Boost duration",
					       String.format("%6.1f s", stats.boost_duration()));
		coast_duration = new MicroStat(layout, y++, "Coast duration",
					       String.format("%6.1f s", stats.coast_duration()));
		descent_speed = new MicroStat(layout, y++, "Descent rate",
					      String.format("%7.1f m/s", stats.descent_speed()),
					      String.format("%7.1f ft/s", AltosConvert.meters_to_feet(stats.descent_speed())));
		descent_duration = new MicroStat(layout, y++, "Descent duration",
						 String.format("%6.1f s", stats.descent_duration()));
		flight_time = new MicroStat(layout, y++, "Flight Time",
					    String.format("%6.1f s", stats.landed_time));
		set_font();

		AltosUIPreferences.register_font_listener(this);
	}

	public void tell_closing() {
		AltosUIPreferences.unregister_font_listener(this);
	}

	public MicroStatsTable() {
		this(new MicroStats());
	}
	
}

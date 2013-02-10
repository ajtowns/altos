/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

package org.altusmetrum.altosuilib_1;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.io.*;
import java.util.concurrent.*;
import java.util.*;
import org.altusmetrum.altoslib_1.*;

import org.jfree.ui.*;
import org.jfree.chart.*;
import org.jfree.chart.plot.*;
import org.jfree.chart.axis.*;
import org.jfree.chart.renderer.*;
import org.jfree.chart.renderer.xy.*;
import org.jfree.chart.labels.*;
import org.jfree.data.xy.*;
import org.jfree.data.*;

public class AltosUIEnable extends Container {

	Insets	il, ir;
	int	y;

	class GraphElement implements ActionListener {
		AltosUIGrapher	grapher;
		JLabel		label;
		JRadioButton	enable;
		String		name;

		public void actionPerformed(ActionEvent ae) {
			grapher.set_enable(enable.isSelected());
		}

		GraphElement (String name, AltosUIGrapher grapher, boolean enabled) {
			this.name = name;
			this.grapher = grapher;
			label = new JLabel(name);
			enable = new JRadioButton("Enable", enabled);
			grapher.set_enable(enabled);			  
			enable.addActionListener(this);
		}
	}

	public void add(String name, AltosUIGrapher grapher, boolean enabled) {

		GraphElement	e = new GraphElement(name, grapher, enabled);

		/* Add label */
		GridBagConstraints c = new GridBagConstraints();
		c.gridx = 0; c.gridy = y;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		add(e.label, c);

		/* Add radio button */
		c = new GridBagConstraints();
		c.gridx = 1; c.gridy = y;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = ir;
		add(e.enable, c);

		/* Next row */
		y++;
	}

	public void add_units() {
		/* Imperial units setting */
		/* Add label */
		GridBagConstraints c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 1000;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		add(new JLabel("Imperial Units"), c);

		JRadioButton imperial_units = new JRadioButton("Enable", AltosUIPreferences.imperial_units());
		imperial_units.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					JRadioButton item = (JRadioButton) e.getSource();
					boolean enabled = item.isSelected();
					AltosUIPreferences.set_imperial_units(enabled);
				}
			});
		imperial_units.setToolTipText("Use Imperial units instead of metric");
		c = new GridBagConstraints();
		c.gridx = 1; c.gridy = 1000;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		add(imperial_units, c);
	}

	public AltosUIEnable() {
		il = new Insets(4,4,4,4);
		ir = new Insets(4,4,4,4);
		y = 0;
		setLayout(new GridBagLayout());
		add_units();
	}
}

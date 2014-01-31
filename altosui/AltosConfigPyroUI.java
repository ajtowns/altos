/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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
import javax.swing.event.*;
import org.altusmetrum.altoslib_3.*;
import org.altusmetrum.altosuilib_1.*;

public class AltosConfigPyroUI
	extends AltosUIDialog
	implements ItemListener, DocumentListener, AltosUnitsListener, ActionListener
{
	AltosConfigUI	owner;
	Container	pane;

	static Insets il = new Insets(4,4,4,4);
	static Insets ir = new Insets(4,4,4,4);

	static String[] state_names;

	static void make_state_names() {
		if (state_names == null) {

			state_names = new String[AltosLib.ao_flight_landed - AltosLib.ao_flight_boost + 1];
			for (int state = AltosLib.ao_flight_boost; state <= AltosLib.ao_flight_landed; state++)
				state_names[state - AltosLib.ao_flight_boost] = AltosLib.state_name_capital(state);
		}
	}

	class PyroItem implements ItemListener, DocumentListener, AltosUnitsListener
	{
		public int		flag;
		public JCheckBox	enable;
		public JTextField	value;
		public JComboBox	combo;
		AltosConfigPyroUI	ui;
		boolean			setting;

		public void set_enable(boolean enable) {
			if (value != null)
				value.setEnabled(enable);
			if (combo != null)
				combo.setEnabled(enable);
		}

		public void itemStateChanged(ItemEvent e) {
			set_enable(enable.isSelected());
			if (!setting) 
				ui.set_dirty();
		}

		public void changedUpdate(DocumentEvent e) {
			if (!setting) 
				ui.set_dirty();
		}

		public void insertUpdate(DocumentEvent e) {
			if (!setting) 
				ui.set_dirty();
		}

		public void removeUpdate(DocumentEvent e) {
			if (!setting) 
				ui.set_dirty();
		}

		public void units_changed(boolean imperial_units) {
			AltosUnits units = AltosPyro.pyro_to_units(flag);

			if (units != null) {
				try {
					double v = units.parse(value.getText(), !imperial_units);
					set(enabled(), v);
				} catch (NumberFormatException ne) {
					set(enabled(), 0.0);
				}
			}
		}

		public void set(boolean new_enable, double new_value) {
			setting = true;
			enable.setSelected(new_enable);
			set_enable(new_enable);
			if (value != null) {
				double	scale = AltosPyro.pyro_to_scale(flag);
				double 	unit_value = new_value;
				AltosUnits units = AltosPyro.pyro_to_units(flag);
				if (units != null)
					unit_value = units.value(new_value);
				String	format = "%6.0f";
				if (scale >= 10)
					format = "%6.1f";
				else if (scale >= 100)
					format = "%6.2f";
				value.setText(String.format(format, unit_value));
			}
			if (combo != null)
				if (new_value >= AltosLib.ao_flight_boost && new_value <= AltosLib.ao_flight_landed)
					combo.setSelectedIndex((int) new_value - AltosLib.ao_flight_boost);
			setting = false;
		}

		public boolean enabled() {
			return enable.isSelected();
		}

		public double value() {
			if (value != null) {
				AltosUnits units = AltosPyro.pyro_to_units(flag);
				if (units != null)
					return units.parse(value.getText());
				return Double.parseDouble(value.getText());
			}
			if (combo != null)
				return combo.getSelectedIndex() + AltosLib.ao_flight_boost;
			return 0;
		}

		public PyroItem(AltosConfigPyroUI in_ui, int in_flag, int x, int y) {

			ui = in_ui;
			flag = in_flag;

			GridBagConstraints	c;
			c = new GridBagConstraints();
			c.gridx = x; c.gridy = y;
			c.gridwidth = 1;
			c.fill = GridBagConstraints.NONE;
			c.anchor = GridBagConstraints.LINE_START;
			c.insets = il;
			enable = new JCheckBox();
			enable.addItemListener(this);
			pane.add(enable, c);
			
			if ((flag & AltosPyro.pyro_no_value) == 0) {
				c = new GridBagConstraints();
				c.gridx = x+1; c.gridy = y;
				c.gridwidth = 1;
				c.fill = GridBagConstraints.NONE;
				c.anchor = GridBagConstraints.LINE_START;
				c.insets = il;
				if ((flag & AltosPyro.pyro_state_value) != 0) {
					make_state_names();
					combo = new JComboBox(state_names);
					combo.addItemListener(this);
					pane.add(combo, c);
				} else {
					value = new JTextField(10);
					value.getDocument().addDocumentListener(this);
					pane.add(value, c);
				}
			}
		}
	}

	class PyroColumn implements AltosUnitsListener {
		public PyroItem[]	items;
		public JLabel		label;
		int			channel;

		public void set(AltosPyro pyro) {
			int row = 0;
			for (int flag = 1; flag < AltosPyro.pyro_all; flag <<= 1) {
				if ((AltosPyro.pyro_all & flag) != 0) {
					items[row].set((pyro.flags & flag) != 0,
						       pyro.get_value(flag));
					row++;
				}
			}
		}

		public AltosPyro get() {
			AltosPyro	p = new AltosPyro(channel);

			int row = 0;
			for (int flag = 1; flag < AltosPyro.pyro_all; flag <<= 1) {
				if ((AltosPyro.pyro_all & flag) != 0) {
					if (items[row].enabled()) {
						p.flags |= flag;
						p.set_value(flag, items[row].value());
					}
					row++;
				}
			}
			return p;
		}

		public void units_changed(boolean imperial_units) {
			int row = 0;
			for (int flag = 1; flag < AltosPyro.pyro_all; flag <<= 1) {
				if ((AltosPyro.pyro_all & flag) != 0) {
					items[row].units_changed(imperial_units);
					row++;
				}
			}
		}

		public PyroColumn(AltosConfigPyroUI ui, int x, int y, int in_channel) {

			channel = in_channel;

			int	nrow = 0;
			for (int flag = 1; flag < AltosPyro.pyro_all; flag <<= 1)
				if ((flag & AltosPyro.pyro_all) != 0)
					nrow++;

			items = new PyroItem[nrow];
			int row = 0;
			
			GridBagConstraints	c;
			c = new GridBagConstraints();
			c.gridx = x; c.gridy = y;
			c.gridwidth = 2;
			c.fill = GridBagConstraints.NONE;
			c.anchor = GridBagConstraints.CENTER;
			c.insets = il;
			label = new JLabel(String.format("Pyro Channel %c", 'A' + channel));
			pane.add(label, c);
			y++;

			for (int flag = 1; flag < AltosPyro.pyro_all; flag <<= 1)
				if ((flag & AltosPyro.pyro_all) != 0) {
					items[row] = new PyroItem(ui, flag, x, y + row);
					row++;
				}
		}
	}

	PyroColumn[]	columns;
	JLabel[]	labels;

	public void set_pyros(AltosPyro[] pyros) {
		for (int i = 0; i < pyros.length; i++) {
			if (pyros[i].channel < columns.length)
				columns[pyros[i].channel].set(pyros[i]);
		}
	}

	public AltosPyro[] get_pyros() {
		AltosPyro[]	pyros = new AltosPyro[columns.length];
		for (int c = 0; c < columns.length; c++)
			pyros[c] = columns[c].get();
		return pyros;
	}

	public void set_dirty() {
		owner.set_dirty();
	}

	public void itemStateChanged(ItemEvent e) {
		owner.set_dirty();
	}

	public void changedUpdate(DocumentEvent e) {
		owner.set_dirty();
	}

	public void insertUpdate(DocumentEvent e) {
		owner.set_dirty();
	}

	public void removeUpdate(DocumentEvent e) {
		owner.set_dirty();
	}

	public void units_changed(boolean imperial_units) {
		for (int c = 0; c < columns.length; c++)
			columns[c].units_changed(imperial_units);
		int r = 0;
		for (int flag = 1; flag <= AltosPyro.pyro_all; flag <<= 1) {
			String n = AltosPyro.pyro_to_name(flag);
			if (n != null) {
				labels[r].setText(n);
				r++;
			}
		}
	}

	/* A window listener to catch closing events and tell the config code */
	class ConfigListener extends WindowAdapter {
		AltosConfigPyroUI	ui;
		AltosConfigUI		owner;

		public ConfigListener(AltosConfigPyroUI this_ui, AltosConfigUI this_owner) {
			ui = this_ui;
			owner = this_owner;
		}

		public void windowClosing(WindowEvent e) {
			ui.setVisible(false);
		}
	}

	/* Listen for events from our buttons */
	public void actionPerformed(ActionEvent e) {
		String	cmd = e.getActionCommand();

		if (cmd.equals("Close"))
			setVisible(false);
	}

	public AltosConfigPyroUI(AltosConfigUI in_owner, AltosPyro[] pyros) {

		super(in_owner, "Configure Pyro Channels", false);

		owner = in_owner;

		GridBagConstraints	c;

		pane = getContentPane();
		pane.setLayout(new GridBagLayout());

		int	nrow = 0;
		for (int flag = 1; flag < AltosPyro.pyro_all; flag <<= 1)
			if ((flag & AltosPyro.pyro_all) != 0)
				nrow++;

		labels = new JLabel[nrow];

		int	row = 1;

		for (int flag = 1; flag <= AltosPyro.pyro_all; flag <<= 1) {
			String	n;

			n = AltosPyro.pyro_to_name(flag);
			if (n != null) {
				c = new GridBagConstraints();
				c.gridx = 0; c.gridy = row;
				c.gridwidth = 1;
				c.fill = GridBagConstraints.NONE;
				c.anchor = GridBagConstraints.LINE_START;
				c.insets = il;
				JLabel label = new JLabel(n);
				pane.add(label, c);
				labels[row-1] = label;
				row++;
			}
		}

		columns = new PyroColumn[pyros.length];

		for (int i = 0; i < pyros.length; i++) {
			columns[i] = new PyroColumn(this, i*2 + 1, 0, i);
			columns[i].set(pyros[i]);
		}

		c = new GridBagConstraints();
		c.gridx = pyros.length*2-1;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridwidth = 2;
		c.gridy = 1000;
		JButton close = new JButton("Close");
		pane.add(close, c);
		close.addActionListener(this);
		close.setActionCommand("Close");
		
		addWindowListener(new ConfigListener(this, owner));
		AltosPreferences.register_units_listener(this);
	}

	public void dispose() {
		AltosPreferences.unregister_units_listener(this);
		super.dispose();
	}

	public void make_visible() {
		pack();
		setVisible(true);
	}
}

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
import javax.swing.event.*;
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

public class AltosConfigTDUI
	extends AltosUIDialog
	implements ActionListener, ItemListener, DocumentListener
{

	Container	pane;
	Box		box;
	JLabel		product_label;
	JLabel		version_label;
	JLabel		serial_label;
	JLabel		frequency_label;
	JLabel		radio_calibration_label;
	JLabel		radio_frequency_label;

	public boolean		dirty;

	JFrame		owner;
	JLabel		product_value;
	JLabel		version_value;
	JLabel		serial_value;
	AltosFreqList	radio_frequency_value;
	JLabel		radio_calibration_value;

	JButton		save;
	JButton		reset;
	JButton		reboot;
	JButton		close;

	ActionListener	listener;


	/* A window listener to catch closing events and tell the config code */
	class ConfigListener extends WindowAdapter {
		AltosConfigTDUI	ui;

		public ConfigListener(AltosConfigTDUI this_ui) {
			ui = this_ui;
		}

		public void windowClosing(WindowEvent e) {
			ui.actionPerformed(new ActionEvent(e.getSource(),
							   ActionEvent.ACTION_PERFORMED,
							   "Close"));
		}
	}

	/* Build the UI using a grid bag */
	public AltosConfigTDUI(JFrame in_owner) {
		super (in_owner, "Configure TeleDongle", false);

		owner = in_owner;
		GridBagConstraints c;

		Insets il = new Insets(4,4,4,4);
		Insets ir = new Insets(4,4,4,4);

		pane = getContentPane();
		pane.setLayout(new GridBagLayout());

		/* Product */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 0;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		product_label = new JLabel("Product:");
		pane.add(product_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = 0;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		product_value = new JLabel("");
		pane.add(product_value, c);

		/* Version */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 1;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		version_label = new JLabel("Software version:");
		pane.add(version_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = 1;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		version_value = new JLabel("");
		pane.add(version_value, c);

		/* Serial */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 2;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		serial_label = new JLabel("Serial:");
		pane.add(serial_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = 2;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		serial_value = new JLabel("");
		pane.add(serial_value, c);

		/* Frequency */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 5;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		radio_frequency_label = new JLabel("Frequency:");
		pane.add(radio_frequency_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = 5;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		radio_frequency_value = new AltosFreqList();
		radio_frequency_value.addItemListener(this);
		pane.add(radio_frequency_value, c);
		radio_frequency_value.setToolTipText("Telemetry, RDF and packet frequency");

		/* Radio Calibration */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 6;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		radio_calibration_label = new JLabel("RF Calibration:");
		pane.add(radio_calibration_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = 6;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		radio_calibration_value = new JLabel(String.format("%d", 1186611));
		pane.add(radio_calibration_value, c);

		/* Buttons */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 12;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		save = new JButton("Save");
		pane.add(save, c);
		save.addActionListener(this);
		save.setActionCommand("Save");

		c = new GridBagConstraints();
		c.gridx = 2; c.gridy = 12;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = il;
		reset = new JButton("Reset");
		pane.add(reset, c);
		reset.addActionListener(this);
		reset.setActionCommand("Reset");

		c = new GridBagConstraints();
		c.gridx = 6; c.gridy = 12;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_END;
		c.insets = il;
		close = new JButton("Close");
		pane.add(close, c);
		close.addActionListener(this);
		close.setActionCommand("Close");

		addWindowListener(new ConfigListener(this));
	}

	/* Once the initial values are set, the config code will show the dialog */
	public void make_visible() {
		pack();
		setLocationRelativeTo(owner);
		setVisible(true);
	}

	/* If any values have been changed, confirm before closing */
	public boolean check_dirty(String operation) {
		if (dirty) {
			Object[] options = { String.format("%s anyway", operation), "Keep editing" };
			int i;
			i = JOptionPane.showOptionDialog(this,
							 String.format("Configuration modified. %s anyway?", operation),
							 "Configuration Modified",
							 JOptionPane.DEFAULT_OPTION,
							 JOptionPane.WARNING_MESSAGE,
							 null, options, options[1]);
			if (i != 0)
				return false;
		}
		return true;
	}

	/* Listen for events from our buttons */
	public void actionPerformed(ActionEvent e) {
		String	cmd = e.getActionCommand();

		if (cmd.equals("Close") || cmd.equals("Reboot"))
			if (!check_dirty(cmd))
				return;
		listener.actionPerformed(e);
		if (cmd.equals("Close") || cmd.equals("Reboot")) {
			setVisible(false);
			dispose();
		}
		dirty = false;
	}

	/* ItemListener interface method */
	public void itemStateChanged(ItemEvent e) {
		dirty = true;
	}

	/* DocumentListener interface methods */
	public void changedUpdate(DocumentEvent e) {
		dirty = true;
	}

	public void insertUpdate(DocumentEvent e) {
		dirty = true;
	}

	public void removeUpdate(DocumentEvent e) {
		dirty = true;
	}

	/* Let the config code hook on a listener */
	public void addActionListener(ActionListener l) {
		listener = l;
	}

	/* set and get all of the dialog values */
	public void set_product(String product) {
		radio_frequency_value.set_product(product);
		product_value.setText(product);
	}

	public void set_version(String version) {
		version_value.setText(version);
	}

	public void set_serial(int serial) {
		radio_frequency_value.set_serial(serial);
		serial_value.setText(String.format("%d", serial));
	}

	public void set_radio_frequency(double new_radio_frequency) {
		int i;
		for (i = 0; i < radio_frequency_value.getItemCount(); i++) {
			AltosFrequency	f = (AltosFrequency) radio_frequency_value.getItemAt(i);

			if (f.close(new_radio_frequency)) {
				radio_frequency_value.setSelectedIndex(i);
				return;
			}
		}
		for (i = 0; i < radio_frequency_value.getItemCount(); i++) {
			AltosFrequency	f = (AltosFrequency) radio_frequency_value.getItemAt(i);

			if (new_radio_frequency < f.frequency)
				break;
		}
		String	description = String.format("%s serial %s",
						    product_value.getText(),
						    serial_value.getText());
		AltosFrequency	new_frequency = new AltosFrequency(new_radio_frequency, description);
		AltosPreferences.add_common_frequency(new_frequency);
		radio_frequency_value.insertItemAt(new_frequency, i);
		radio_frequency_value.setSelectedIndex(i);
	}

	public double radio_frequency() {
		return radio_frequency_value.frequency();
	}

	public void set_radio_calibration(int calibration) {
		radio_calibration_value.setText(String.format("%d", calibration));
	}

	public void set_clean() {
		dirty = false;
	}
}

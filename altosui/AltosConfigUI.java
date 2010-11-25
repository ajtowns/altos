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
import javax.swing.event.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.util.concurrent.LinkedBlockingQueue;

import libaltosJNI.*;

public class AltosConfigUI
	extends JDialog
	implements ActionListener, ItemListener, DocumentListener
{

	Container	pane;
	Box		box;
	JLabel		product_label;
	JLabel		version_label;
	JLabel		serial_label;
	JLabel		main_deploy_label;
	JLabel		apogee_delay_label;
	JLabel		radio_channel_label;
	JLabel		radio_calibration_label;
	JLabel		callsign_label;

	public boolean		dirty;

	JFrame		owner;
	JLabel		product_value;
	JLabel		version_value;
	JLabel		serial_value;
	JComboBox	main_deploy_value;
	JComboBox	apogee_delay_value;
	JComboBox	radio_channel_value;
	JTextField	radio_calibration_value;
	JTextField	callsign_value;

	JButton		save;
	JButton		reset;
	JButton		reboot;
	JButton		close;

	ActionListener	listener;

	static String[] main_deploy_values = {
		"100", "150", "200", "250", "300", "350",
		"400", "450", "500"
	};

	static String[] apogee_delay_values = {
		"0", "1", "2", "3", "4", "5"
	};

	static String[] radio_channel_values = new String[10];
		{
			for (int i = 0; i <= 9; i++)
				radio_channel_values[i] = String.format("Channel %1d (%7.3fMHz)",
									i, 434.550 + i * 0.1);
		}

	/* A window listener to catch closing events and tell the config code */
	class ConfigListener extends WindowAdapter {
		AltosConfigUI	ui;

		public ConfigListener(AltosConfigUI this_ui) {
			ui = this_ui;
		}

		public void windowClosing(WindowEvent e) {
			ui.actionPerformed(new ActionEvent(e.getSource(),
							   ActionEvent.ACTION_PERFORMED,
							   "Close"));
		}
	}

	/* Build the UI using a grid bag */
	public AltosConfigUI(JFrame in_owner, boolean remote) {
		super (in_owner, "Configure TeleMetrum", false);

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

		/* Main deploy */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 3;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		main_deploy_label = new JLabel("Main Deploy Altitude(m):");
		pane.add(main_deploy_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = 3;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		main_deploy_value = new JComboBox(main_deploy_values);
		main_deploy_value.setEditable(true);
		main_deploy_value.addItemListener(this);
		pane.add(main_deploy_value, c);

		/* Apogee delay */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 4;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		apogee_delay_label = new JLabel("Apogee Delay(s):");
		pane.add(apogee_delay_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = 4;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		apogee_delay_value = new JComboBox(apogee_delay_values);
		apogee_delay_value.setEditable(true);
		apogee_delay_value.addItemListener(this);
		pane.add(apogee_delay_value, c);

		/* Radio channel */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 5;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		radio_channel_label = new JLabel("Radio Channel:");
		pane.add(radio_channel_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = 5;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		radio_channel_value = new JComboBox(radio_channel_values);
		radio_channel_value.setEditable(false);
		radio_channel_value.addItemListener(this);
		if (remote)
			radio_channel_value.setEnabled(false);
		pane.add(radio_channel_value, c);

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
		radio_calibration_value = new JTextField(String.format("%d", 1186611));
		radio_calibration_value.getDocument().addDocumentListener(this);
		if (remote)
			radio_calibration_value.setEnabled(false);
		pane.add(radio_calibration_value, c);

		/* Callsign */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 7;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		callsign_label = new JLabel("Callsign:");
		pane.add(callsign_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = 7;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		callsign_value = new JTextField(AltosPreferences.callsign());
		callsign_value.getDocument().addDocumentListener(this);
		pane.add(callsign_value, c);

		/* Buttons */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 8;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		save = new JButton("Save");
		pane.add(save, c);
		save.addActionListener(this);
		save.setActionCommand("Save");

		c = new GridBagConstraints();
		c.gridx = 2; c.gridy = 8;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = il;
		reset = new JButton("Reset");
		pane.add(reset, c);
		reset.addActionListener(this);
		reset.setActionCommand("Reset");

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = 8;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = il;
		reboot = new JButton("Reboot");
		pane.add(reboot, c);
		reboot.addActionListener(this);
		reboot.setActionCommand("Reboot");

		c = new GridBagConstraints();
		c.gridx = 6; c.gridy = 8;
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
		product_value.setText(product);
	}

	public void set_version(String version) {
		version_value.setText(version);
	}

	public void set_serial(int serial) {
		serial_value.setText(String.format("%d", serial));
	}

	public void set_main_deploy(int new_main_deploy) {
		main_deploy_value.setSelectedItem(Integer.toString(new_main_deploy));
	}

	public int main_deploy() {
		return Integer.parseInt(main_deploy_value.getSelectedItem().toString());
	}

	public void set_apogee_delay(int new_apogee_delay) {
		apogee_delay_value.setSelectedItem(Integer.toString(new_apogee_delay));
	}

	public int apogee_delay() {
		return Integer.parseInt(apogee_delay_value.getSelectedItem().toString());
	}

	public void set_radio_channel(int new_radio_channel) {
		radio_channel_value.setSelectedIndex(new_radio_channel);
	}

	public int radio_channel() {
		return radio_channel_value.getSelectedIndex();
	}

	public void set_radio_calibration(int new_radio_calibration) {
		radio_calibration_value.setText(String.format("%d", new_radio_calibration));
	}

	public int radio_calibration() {
		return Integer.parseInt(radio_calibration_value.getText());
	}

	public void set_callsign(String new_callsign) {
		callsign_value.setText(new_callsign);
	}

	public String callsign() {
		return callsign_value.getText();
	}

	public void set_clean() {
		dirty = false;
	}

 }
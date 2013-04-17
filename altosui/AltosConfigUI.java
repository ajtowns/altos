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
import org.altusmetrum.altoslib_1.*;
import org.altusmetrum.altosuilib_1.*;

public class AltosConfigUI
	extends AltosUIDialog
	implements ActionListener, ItemListener, DocumentListener, AltosConfigValues
{

	Container	pane;
	JLabel		product_label;
	JLabel		version_label;
	JLabel		serial_label;
	JLabel		main_deploy_label;
	JLabel		apogee_delay_label;
	JLabel		apogee_lockout_label;
	JLabel		frequency_label;
	JLabel		radio_calibration_label;
	JLabel		radio_frequency_label;
	JLabel		radio_enable_label;
	JLabel		aprs_interval_label;
	JLabel		flight_log_max_label;
	JLabel		ignite_mode_label;
	JLabel		pad_orientation_label;
	JLabel		callsign_label;

	public boolean		dirty;

	JFrame		owner;
	JLabel		product_value;
	JLabel		version_value;
	JLabel		serial_value;
	JComboBox	main_deploy_value;
	JComboBox	apogee_delay_value;
	JComboBox	apogee_lockout_value;
	AltosFreqList	radio_frequency_value;
	JTextField	radio_calibration_value;
	JRadioButton	radio_enable_value;
	JComboBox	aprs_interval_value;
	JComboBox	flight_log_max_value;
	JComboBox	ignite_mode_value;
	JComboBox	pad_orientation_value;
	JTextField	callsign_value;

	JButton		pyro;

	JButton		save;
	JButton		reset;
	JButton		reboot;
	JButton		close;

	AltosPyro[]	pyros;

	ActionListener	listener;

	static String[] main_deploy_values = {
		"100", "150", "200", "250", "300", "350",
		"400", "450", "500"
	};

	static String[] apogee_delay_values = {
		"0", "1", "2", "3", "4", "5"
	};

	static String[] apogee_lockout_values = {
		"0", "5", "10", "15", "20"
	};

	static String[] flight_log_max_values = {
		"64", "128", "192", "256", "320",
		"384", "448", "512", "576", "640",
		"704", "768", "832", "896", "960",
	};

	static String[] ignite_mode_values = {
		"Dual Deploy",
		"Redundant Apogee",
		"Redundant Main",
	};

	static String[] aprs_interval_values = {
		"Disabled",
		"2",
		"5",
		"10"
	};

	static String[] pad_orientation_values = {
		"Antenna Up",
		"Antenna Down",
	};

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

	boolean is_telemini() {
		String	product = product_value.getText();
		return product != null && product.startsWith("TeleMini");
	}

	boolean is_telemetrum() {
		String	product = product_value.getText();
		return product != null && product.startsWith("TeleMetrum");
	}

	void set_radio_calibration_tool_tip() {
		if (radio_calibration_value.isEnabled())
			radio_calibration_value.setToolTipText("Tune radio output to match desired frequency");
		else
			radio_calibration_value.setToolTipText("Cannot tune radio while connected over packet mode");
	}

	void set_radio_enable_tool_tip() {
		if (radio_enable_value.isEnabled())
			radio_enable_value.setToolTipText("Enable/Disable telemetry and RDF transmissions");
		else
			radio_enable_value.setToolTipText("Firmware version does not support disabling radio");
	}

	void set_aprs_interval_tool_tip() {
		if (aprs_interval_value.isEnabled())
			aprs_interval_value.setToolTipText("Enable APRS and set the interval between APRS reports");
		else
			aprs_interval_value.setToolTipText("Hardware doesn't support APRS");
	}

	void set_flight_log_max_tool_tip() {
		if (flight_log_max_value.isEnabled())
			flight_log_max_value.setToolTipText("Size reserved for each flight log (in kB)");
		else {
			if (is_telemetrum())
				flight_log_max_value.setToolTipText("Cannot set max value with flight logs in memory");
			else if (is_telemini())
				flight_log_max_value.setToolTipText("TeleMini stores only one flight");
			else
				flight_log_max_value.setToolTipText("Cannot set max flight log value");
		}
	}

	void set_ignite_mode_tool_tip() {
		if (ignite_mode_value.isEnabled())
			ignite_mode_value.setToolTipText("Select when igniters will be fired");
		else
			ignite_mode_value.setToolTipText("Older firmware could not select ignite mode");
	}

	void set_pad_orientation_tool_tip() {
		if (pad_orientation_value.isEnabled())
			pad_orientation_value.setToolTipText("How will TeleMetrum be mounted in the airframe");
		else {
			if (is_telemetrum())
				pad_orientation_value.setToolTipText("Older TeleMetrum firmware must fly antenna forward");
			else if (is_telemini())
				pad_orientation_value.setToolTipText("TeleMini doesn't care how it is mounted");
			else
				pad_orientation_value.setToolTipText("Can't select orientation");
		}
	}

	/* Build the UI using a grid bag */
	public AltosConfigUI(JFrame in_owner, boolean remote) {
		super (in_owner, "Configure TeleMetrum", false);

		owner = in_owner;
		GridBagConstraints c;
		int row = 0;

		Insets il = new Insets(4,4,4,4);
		Insets ir = new Insets(4,4,4,4);

		pane = getContentPane();
		pane.setLayout(new GridBagLayout());

		/* Product */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		product_label = new JLabel("Product:");
		pane.add(product_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		product_value = new JLabel("");
		pane.add(product_value, c);
		row++;

		/* Version */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		version_label = new JLabel("Software version:");
		pane.add(version_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		version_value = new JLabel("");
		pane.add(version_value, c);
		row++;

		/* Serial */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		serial_label = new JLabel("Serial:");
		pane.add(serial_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		serial_value = new JLabel("");
		pane.add(serial_value, c);
		row++;

		/* Main deploy */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		main_deploy_label = new JLabel("Main Deploy Altitude(m):");
		pane.add(main_deploy_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
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
		main_deploy_value.setToolTipText("Height above pad altitude to fire main charge");
		row++;

		/* Apogee delay */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		apogee_delay_label = new JLabel("Apogee Delay(s):");
		pane.add(apogee_delay_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
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
		apogee_delay_value.setToolTipText("Delay after apogee before charge fires");
		row++;

		/* Apogee lockout */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		apogee_lockout_label = new JLabel("Apogee Lockout(s):");
		pane.add(apogee_lockout_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		apogee_lockout_value = new JComboBox(apogee_lockout_values);
		apogee_lockout_value.setEditable(true);
		apogee_lockout_value.addItemListener(this);
		pane.add(apogee_lockout_value, c);
		apogee_lockout_value.setToolTipText("Time after boost while apogee detection is locked out");
		row++;

		/* Frequency */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		radio_frequency_label = new JLabel("Frequency:");
		pane.add(radio_frequency_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
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
		row++;

		/* Radio Calibration */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		radio_calibration_label = new JLabel("RF Calibration:");
		pane.add(radio_calibration_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
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
		set_radio_calibration_tool_tip();
		row++;

		/* Radio Enable */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		radio_enable_label = new JLabel("Telemetry/RDF/APRS Enable:");
		pane.add(radio_enable_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		radio_enable_value = new JRadioButton("Enabled");
		radio_enable_value.addItemListener(this);
		pane.add(radio_enable_value, c);
		set_radio_enable_tool_tip();
		row++;

		/* APRS interval */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		aprs_interval_label = new JLabel("APRS Interval(s):");
		pane.add(aprs_interval_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		aprs_interval_value = new JComboBox(aprs_interval_values);
		aprs_interval_value.setEditable(true);
		aprs_interval_value.addItemListener(this);
		pane.add(aprs_interval_value, c);
		set_aprs_interval_tool_tip();
		row++;

		/* Callsign */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		callsign_label = new JLabel("Callsign:");
		pane.add(callsign_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		callsign_value = new JTextField(AltosUIPreferences.callsign());
		callsign_value.getDocument().addDocumentListener(this);
		pane.add(callsign_value, c);
		callsign_value.setToolTipText("Callsign reported in telemetry data");
		row++;

		/* Flight log max */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		flight_log_max_label = new JLabel("Maximum Flight Log Size:");
		pane.add(flight_log_max_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		flight_log_max_value = new JComboBox(flight_log_max_values);
		flight_log_max_value.setEditable(true);
		flight_log_max_value.addItemListener(this);
		pane.add(flight_log_max_value, c);
		set_flight_log_max_tool_tip();
		row++;

		/* Ignite mode */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		ignite_mode_label = new JLabel("Igniter Firing Mode:");
		pane.add(ignite_mode_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		ignite_mode_value = new JComboBox(ignite_mode_values);
		ignite_mode_value.setEditable(false);
		ignite_mode_value.addItemListener(this);
		pane.add(ignite_mode_value, c);
		set_ignite_mode_tool_tip();
		row++;

		/* Pad orientation */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		pad_orientation_label = new JLabel("Pad Orientation:");
		pane.add(pad_orientation_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		pad_orientation_value = new JComboBox(pad_orientation_values);
		pad_orientation_value.setEditable(false);
		pad_orientation_value.addItemListener(this);
		pane.add(pad_orientation_value, c);
		set_pad_orientation_tool_tip();
		row++;

		/* Pyro channels */
		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		pyro = new JButton("Configure Pyro Channels");
		pane.add(pyro, c);
		pyro.addActionListener(this);
		pyro.setActionCommand("Pyro");
		row++;

		/* Buttons */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		save = new JButton("Save");
		pane.add(save, c);
		save.addActionListener(this);
		save.setActionCommand("Save");

		c = new GridBagConstraints();
		c.gridx = 2; c.gridy = row;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = il;
		reset = new JButton("Reset");
		pane.add(reset, c);
		reset.addActionListener(this);
		reset.setActionCommand("Reset");

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = il;
		reboot = new JButton("Reboot");
		pane.add(reboot, c);
		reboot.addActionListener(this);
		reboot.setActionCommand("Reboot");

		c = new GridBagConstraints();
		c.gridx = 6; c.gridy = row;
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

	void set_dirty() {
		dirty = true;
		save.setEnabled(true);
	}

	public void set_clean() {
		dirty = false;
		save.setEnabled(false);
	}

	AltosConfigPyroUI	pyro_ui;

	/* Listen for events from our buttons */
	public void actionPerformed(ActionEvent e) {
		String	cmd = e.getActionCommand();

		if (cmd.equals("Pyro")) {
			if (pyro_ui == null && pyros != null) {
				pyro_ui = new AltosConfigPyroUI(this, pyros);
				pyro_ui.make_visible();
			}
			return;
		}

		if (cmd.equals("Close") || cmd.equals("Reboot"))
			if (!check_dirty(cmd))
				return;
		listener.actionPerformed(e);
		if (cmd.equals("Close") || cmd.equals("Reboot")) {
			setVisible(false);
			dispose();
		}
		set_clean();
	}

	/* ItemListener interface method */
	public void itemStateChanged(ItemEvent e) {
		set_dirty();
	}

	/* DocumentListener interface methods */
	public void changedUpdate(DocumentEvent e) {
		set_dirty();
	}

	public void insertUpdate(DocumentEvent e) {
		set_dirty();
	}

	public void removeUpdate(DocumentEvent e) {
		set_dirty();
	}

	/* Let the config code hook on a listener */
	public void addActionListener(ActionListener l) {
		listener = l;
	}

	/* set and get all of the dialog values */
	public void set_product(String product) {
		radio_frequency_value.set_product(product);
		product_value.setText(product);
		set_pad_orientation_tool_tip();
		set_flight_log_max_tool_tip();
	}

	public void set_version(String version) {
		version_value.setText(version);
	}

	public void set_serial(int serial) {
		radio_frequency_value.set_serial(serial);
		serial_value.setText(String.format("%d", serial));
	}

	public void set_main_deploy(int new_main_deploy) {
		main_deploy_value.setSelectedItem(Integer.toString(new_main_deploy));
		main_deploy_value.setEnabled(new_main_deploy >= 0);
	}

	public int main_deploy() {
		return Integer.parseInt(main_deploy_value.getSelectedItem().toString());
	}

	public void set_apogee_delay(int new_apogee_delay) {
		apogee_delay_value.setSelectedItem(Integer.toString(new_apogee_delay));
		apogee_delay_value.setEnabled(new_apogee_delay >= 0);
	}

	public int apogee_delay() {
		return Integer.parseInt(apogee_delay_value.getSelectedItem().toString());
	}

	public void set_apogee_lockout(int new_apogee_lockout) {
		apogee_lockout_value.setSelectedItem(Integer.toString(new_apogee_lockout));
		apogee_lockout_value.setEnabled(new_apogee_lockout >= 0);
	}

	public int apogee_lockout() {
		return Integer.parseInt(apogee_lockout_value.getSelectedItem().toString());
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
		AltosUIPreferences.add_common_frequency(new_frequency);
		radio_frequency_value.insertItemAt(new_frequency, i);
		radio_frequency_value.setSelectedIndex(i);
	}

	public double radio_frequency() {
		return radio_frequency_value.frequency();
	}

	public void set_radio_calibration(int new_radio_calibration) {
		radio_calibration_value.setText(String.format("%d", new_radio_calibration));
	}

	public int radio_calibration() {
		return Integer.parseInt(radio_calibration_value.getText());
	}

	public void set_radio_enable(int new_radio_enable) {
		if (new_radio_enable >= 0) {
			radio_enable_value.setSelected(new_radio_enable > 0);
			radio_enable_value.setEnabled(true);
		} else {
			radio_enable_value.setSelected(true);
			radio_enable_value.setEnabled(false);
		}
		set_radio_enable_tool_tip();
	}

	public int radio_enable() {
		if (radio_enable_value.isEnabled())
			return radio_enable_value.isSelected() ? 1 : 0;
		else
			return -1;
	}

	public void set_callsign(String new_callsign) {
		callsign_value.setText(new_callsign);
	}

	public String callsign() {
		return callsign_value.getText();
	}

	public void set_flight_log_max(int new_flight_log_max) {
		flight_log_max_value.setEnabled(new_flight_log_max > 0);
		flight_log_max_value.setSelectedItem(Integer.toString(new_flight_log_max));
		set_flight_log_max_tool_tip();
	}

	public void set_flight_log_max_enabled(boolean enable) {
		flight_log_max_value.setEnabled(enable);
		set_flight_log_max_tool_tip();
	}

	public int flight_log_max() {
		return Integer.parseInt(flight_log_max_value.getSelectedItem().toString());
	}

	public void set_flight_log_max_limit(int flight_log_max_limit) {
		//boolean	any_added = false;
		flight_log_max_value.removeAllItems();
		for (int i = 0; i < flight_log_max_values.length; i++) {
			if (Integer.parseInt(flight_log_max_values[i]) < flight_log_max_limit){
				flight_log_max_value.addItem(flight_log_max_values[i]);
				//any_added = true;
			}
		}
		flight_log_max_value.addItem(String.format("%d", flight_log_max_limit));
	}

	public void set_ignite_mode(int new_ignite_mode) {
		if (new_ignite_mode >= ignite_mode_values.length)
			new_ignite_mode = 0;
		if (new_ignite_mode < 0) {
			ignite_mode_value.setEnabled(false);
			new_ignite_mode = 0;
		} else {
			ignite_mode_value.setEnabled(true);
		}
		ignite_mode_value.setSelectedIndex(new_ignite_mode);
		set_ignite_mode_tool_tip();
	}

	public int ignite_mode() {
		if (ignite_mode_value.isEnabled())
			return ignite_mode_value.getSelectedIndex();
		else
			return -1;
	}


	public void set_pad_orientation(int new_pad_orientation) {
		if (new_pad_orientation >= pad_orientation_values.length)
			new_pad_orientation = 0;
		if (new_pad_orientation < 0) {
			pad_orientation_value.setEnabled(false);
			new_pad_orientation = 0;
		} else {
			pad_orientation_value.setEnabled(true);
		}
		pad_orientation_value.setSelectedIndex(new_pad_orientation);
		set_pad_orientation_tool_tip();
	}

	public int pad_orientation() {
		if (pad_orientation_value.isEnabled())
			return pad_orientation_value.getSelectedIndex();
		else
			return -1;
	}

	public void set_pyros(AltosPyro[] new_pyros) {
		pyros = new_pyros;
		pyro.setEnabled(pyros != null);
		if (pyros != null && pyro_ui != null)
			pyro_ui.set_pyros(pyros);
	}

	public AltosPyro[] pyros() {
		if (pyro_ui != null)
			pyros = pyro_ui.get_pyros();
		return pyros;
	}

	public void set_aprs_interval(int new_aprs_interval) {
		String	s;

		if (new_aprs_interval <= 0)
			s = "Disabled";
		else
			s = Integer.toString(new_aprs_interval);
		aprs_interval_value.setSelectedItem(s);
		aprs_interval_value.setEnabled(new_aprs_interval >= 0);
		set_aprs_interval_tool_tip();
	}

	public int aprs_interval() {
		String	s = aprs_interval_value.getSelectedItem().toString();

		if (s.equals("Disabled"))
			return 0;
		return Integer.parseInt(s);
	}
}

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

public class AltosConfigUI
	extends AltosUIDialog
	implements ActionListener, ItemListener, DocumentListener, AltosConfigValues, AltosUnitsListener
{

	Container		pane;
	JLabel			product_label;
	JLabel			version_label;
	JLabel			serial_label;
	JLabel			main_deploy_label;
	JLabel			apogee_delay_label;
	JLabel			apogee_lockout_label;
	JLabel			frequency_label;
	JLabel			radio_calibration_label;
	JLabel			radio_frequency_label;
	JLabel			radio_enable_label;
	JLabel			aprs_interval_label;
	JLabel			flight_log_max_label;
	JLabel			ignite_mode_label;
	JLabel			pad_orientation_label;
	JLabel			callsign_label;
	JLabel			beep_label;
	JLabel			tracker_motion_label;
	JLabel			tracker_interval_label;

	public boolean		dirty;

	JFrame			owner;
	JLabel			product_value;
	JLabel			version_value;
	JLabel			serial_value;
	JComboBox<String>	main_deploy_value;
	JComboBox<String>	apogee_delay_value;
	JComboBox<String>	apogee_lockout_value;
	AltosFreqList		radio_frequency_value;
	JTextField		radio_calibration_value;
	JRadioButton		radio_enable_value;
	JComboBox<String>	aprs_interval_value;
	JComboBox<String>	flight_log_max_value;
	JComboBox<String>	ignite_mode_value;
	JComboBox<String>	pad_orientation_value;
	JTextField		callsign_value;
	JComboBox<String>	beep_value;
	JComboBox<String>	tracker_motion_value;
	JComboBox<String>	tracker_interval_value;

	JButton			pyro;

	JButton			save;
	JButton			reset;
	JButton			reboot;
	JButton			close;

	AltosPyro[]		pyros;
	double			pyro_firing_time;

	ActionListener		listener;

	static String[] 	main_deploy_values_m = {
		"100", "150", "200", "250", "300", "350",
		"400", "450", "500"
	};

	static String[] 	main_deploy_values_ft = {
		"250", "500", "750", "1000", "1250", "1500",
		"1750", "2000"
	};

	static String[] 	apogee_delay_values = {
		"0", "1", "2", "3", "4", "5"
	};

	static String[] 	apogee_lockout_values = {
		"0", "5", "10", "15", "20"
	};

	static String[] 	ignite_mode_values = {
		"Dual Deploy",
		"Redundant Apogee",
		"Redundant Main",
	};

	static String[] 	aprs_interval_values = {
		"Disabled",
		"2",
		"5",
		"10"
	};

	static String[] 	beep_values = {
		"3750",
		"4000",
		"4250",
	};

	static String[] 	pad_orientation_values = {
		"Antenna Up",
		"Antenna Down",
	};

	static String[]		tracker_motion_values_m = {
		"2",
		"5",
		"10",
		"25",
	};

	static String[]		tracker_motion_values_ft = {
		"5",
		"20",
		"50",
		"100"
	};

	static String[]		tracker_interval_values = {
		"1",
		"2",
		"5",
		"10"
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

	boolean is_telemini_v1() {
		String	product = product_value.getText();
		return product != null && product.startsWith("TeleMini-v1");
	}

	boolean is_telemini() {
		String	product = product_value.getText();
		return product != null && product.startsWith("TeleMini");
	}

	boolean is_easymini() {
		String	product = product_value.getText();
		return product != null && product.startsWith("EasyMini");
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
			if (is_telemini_v1())
				flight_log_max_value.setToolTipText("TeleMini-v1 stores only one flight");
			else
				flight_log_max_value.setToolTipText("Cannot set max value with flight logs in memory");
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
			pad_orientation_value.setToolTipText("How will the computer be mounted in the airframe");
		else {
			if (is_telemetrum())
				pad_orientation_value.setToolTipText("Older TeleMetrum firmware must fly antenna forward");
			else if (is_telemini() || is_easymini())
				pad_orientation_value.setToolTipText("TeleMini and EasyMini don't care how they are mounted");
			else
				pad_orientation_value.setToolTipText("Can't select orientation");
		}
	}

	void set_beep_tool_tip() {
		if (beep_value.isEnabled())
			beep_value.setToolTipText("What frequency the beeper will sound at");
		else
			beep_value.setToolTipText("Older firmware could not select beeper frequency");
	}

	/* Build the UI using a grid bag */
	public AltosConfigUI(JFrame in_owner, boolean remote) {
		super (in_owner, "Configure Flight Computer", false);

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
		main_deploy_label = new JLabel(get_main_deploy_label());
		pane.add(main_deploy_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		main_deploy_value = new JComboBox<String>(main_deploy_values());
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
		apogee_delay_value = new JComboBox<String>(apogee_delay_values);
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
		apogee_lockout_value = new JComboBox<String>(apogee_lockout_values);
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
		aprs_interval_value = new JComboBox<String>(aprs_interval_values);
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
		flight_log_max_label = new JLabel("Maximum Flight Log Size (kB):");
		pane.add(flight_log_max_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		flight_log_max_value = new JComboBox<String>();
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
		ignite_mode_value = new JComboBox<String>(ignite_mode_values);
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
		pad_orientation_value = new JComboBox<String>(pad_orientation_values);
		pad_orientation_value.setEditable(false);
		pad_orientation_value.addItemListener(this);
		pane.add(pad_orientation_value, c);
		set_pad_orientation_tool_tip();
		row++;

		/* Beeper */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		beep_label = new JLabel("Beeper Frequency:");
		pane.add(beep_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		beep_value = new JComboBox<String>(beep_values);
		beep_value.setEditable(true);
		beep_value.addItemListener(this);
		pane.add(beep_value, c);
		set_beep_tool_tip();
		row++;

		/* Tracker triger horiz distances */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		tracker_motion_label = new JLabel(get_tracker_motion_label());
		pane.add(tracker_motion_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		tracker_motion_value = new JComboBox<String>(tracker_motion_values());
		tracker_motion_value.setEditable(true);
		tracker_motion_value.addItemListener(this);
		pane.add(tracker_motion_value, c);
		row++;

		/* Tracker triger vert distances */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		tracker_interval_label = new JLabel("Position Reporting Interval(s):");
		pane.add(tracker_interval_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		tracker_interval_value = new JComboBox<String>(tracker_interval_values);
		tracker_interval_value.setEditable(true);
		tracker_interval_value.addItemListener(this);
		pane.add(tracker_interval_value, c);
		set_tracker_tool_tip();
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
		AltosPreferences.register_units_listener(this);
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

	public void dispose() {
		if (pyro_ui != null)
			pyro_ui.dispose();
		AltosPreferences.unregister_units_listener(this);
		super.dispose();
	}

	/* Listen for events from our buttons */
	public void actionPerformed(ActionEvent e) {
		String	cmd = e.getActionCommand();

		if (cmd.equals("Pyro")) {
			if (pyro_ui == null && pyros != null)
				pyro_ui = new AltosConfigPyroUI(this, pyros, pyro_firing_time);
			if (pyro_ui != null)
				pyro_ui.make_visible();
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
		main_deploy_value.setSelectedItem(AltosConvert.height.say(new_main_deploy));
		main_deploy_value.setEnabled(new_main_deploy >= 0);
	}

	public int main_deploy() {
		return (int) (AltosConvert.height.parse(main_deploy_value.getSelectedItem().toString()) + 0.5);
	}

	String get_main_deploy_label() {
		return String.format("Main Deploy Altitude(%s):", AltosConvert.height.show_units());
	}

	String[] main_deploy_values() {
		if (AltosConvert.imperial_units)
			return main_deploy_values_ft;
		else
			return main_deploy_values_m;
	}

	void set_main_deploy_values() {
		String[]	v = main_deploy_values();
		while (main_deploy_value.getItemCount() > 0)
			main_deploy_value.removeItemAt(0);
		for (int i = 0; i < v.length; i++)
			main_deploy_value.addItem(v[i]);
		main_deploy_value.setMaximumRowCount(v.length);
	}

	public void units_changed(boolean imperial_units) {
		String v = main_deploy_value.getSelectedItem().toString();
		main_deploy_label.setText(get_main_deploy_label());
		set_main_deploy_values();
		int m = (int) (AltosConvert.height.parse(v, !imperial_units) + 0.5);
		set_main_deploy(m);

		if (tracker_motion_value.isEnabled()) {
			String motion = tracker_motion_value.getSelectedItem().toString();
			tracker_motion_label.setText(get_tracker_motion_label());
			set_tracker_motion_values();
			set_tracker_motion((int) (AltosConvert.height.parse(motion, !imperial_units) + 0.5));
		}
	}

	public void set_apogee_delay(int new_apogee_delay) {
		apogee_delay_value.setSelectedItem(Integer.toString(new_apogee_delay));
		apogee_delay_value.setEnabled(new_apogee_delay >= 0);
	}

	private int parse_int(String name, String s, boolean split) throws AltosConfigDataException {
		String v = s;
		if (split)
			v = s.split("\\s+")[0];
		try {
			return Integer.parseInt(v);
		} catch (NumberFormatException ne) {
			throw new AltosConfigDataException("Invalid %s \"%s\"", name, s);
		}
	}

	public int apogee_delay() throws AltosConfigDataException {
		return parse_int("apogee delay", apogee_delay_value.getSelectedItem().toString(), false);
	}

	public void set_apogee_lockout(int new_apogee_lockout) {
		apogee_lockout_value.setSelectedItem(Integer.toString(new_apogee_lockout));
		apogee_lockout_value.setEnabled(new_apogee_lockout >= 0);
	}

	public int apogee_lockout() throws AltosConfigDataException {
		return parse_int("apogee lockout", apogee_lockout_value.getSelectedItem().toString(), false);
	}

	public void set_radio_frequency(double new_radio_frequency) {
		radio_frequency_value.set_frequency(new_radio_frequency);
	}

	public double radio_frequency() {
		return radio_frequency_value.frequency();
	}

	public void set_radio_calibration(int new_radio_calibration) {
		radio_calibration_value.setVisible(new_radio_calibration >= 0);
		if (new_radio_calibration < 0)
			radio_calibration_value.setText("Disabled");
		else
			radio_calibration_value.setText(String.format("%d", new_radio_calibration));
	}

	public int radio_calibration() throws AltosConfigDataException {
		return parse_int("radio calibration", radio_calibration_value.getText(), false);
	}

	public void set_radio_enable(int new_radio_enable) {
		if (new_radio_enable >= 0) {
			radio_enable_value.setSelected(new_radio_enable > 0);
			radio_enable_value.setEnabled(true);
		} else {
			radio_enable_value.setSelected(true);
			radio_enable_value.setVisible(radio_frequency() > 0);
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
		callsign_value.setVisible(new_callsign != null);
		callsign_value.setText(new_callsign);
	}

	public String callsign() {
		return callsign_value.getText();
	}

	int	flight_log_max_limit;
	int	flight_log_max;

	public String flight_log_max_label(int flight_log_max) {
		if (flight_log_max_limit != 0) {
			int	nflight = flight_log_max_limit / flight_log_max;
			String	plural = nflight > 1 ? "s" : "";

			return String.format("%d (%d flight%s)", flight_log_max, nflight, plural);
		}
		return String.format("%d", flight_log_max);
	}

	public void set_flight_log_max(int new_flight_log_max) {
		flight_log_max_value.setSelectedItem(flight_log_max_label(new_flight_log_max));
		flight_log_max = new_flight_log_max;
		set_flight_log_max_tool_tip();
	}

	public void set_flight_log_max_enabled(boolean enable) {
		flight_log_max_value.setEnabled(enable);
		set_flight_log_max_tool_tip();
	}

	public int flight_log_max() throws AltosConfigDataException {
		return parse_int("flight log max", flight_log_max_value.getSelectedItem().toString(), true);
	}

	public void set_flight_log_max_limit(int new_flight_log_max_limit) {
		flight_log_max_limit = new_flight_log_max_limit;
		flight_log_max_value.removeAllItems();
		for (int i = 8; i >= 1; i--) {
			int	size = flight_log_max_limit / i;
			flight_log_max_value.addItem(String.format("%d (%d flights)", size, i));
		}
		if (flight_log_max != 0)
			set_flight_log_max(flight_log_max);
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
			pad_orientation_value.setVisible(false);
			new_pad_orientation = 0;
		} else {
			pad_orientation_value.setVisible(true);
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

	public void set_beep(int new_beep) {
		int new_freq = (int) Math.floor (AltosConvert.beep_value_to_freq(new_beep) + 0.5);
		for (int i = 0; i < beep_values.length; i++)
			if (new_beep == AltosConvert.beep_freq_to_value(Integer.parseInt(beep_values[i]))) {
				beep_value.setSelectedIndex(i);
				set_beep_tool_tip();
				return;
			}
		beep_value.setSelectedItem(String.format("%d", new_freq));
		beep_value.setEnabled(new_beep >= 0);
		set_beep_tool_tip();
	}

	public int beep() {
		if (beep_value.isEnabled())
			return AltosConvert.beep_freq_to_value(Integer.parseInt(beep_value.getSelectedItem().toString()));
		else
			return -1;
	}

	String[] tracker_motion_values() {
		if (AltosConvert.imperial_units)
			return tracker_motion_values_ft;
		else
			return tracker_motion_values_m;
	}

	void set_tracker_motion_values() {
		String[]	v = tracker_motion_values();
		while (tracker_motion_value.getItemCount() > 0)
			tracker_motion_value.removeItemAt(0);
		for (int i = 0; i < v.length; i++)
			tracker_motion_value.addItem(v[i]);
		tracker_motion_value.setMaximumRowCount(v.length);
	}

	String get_tracker_motion_label() {
		return String.format("Logging Trigger Motion (%s):", AltosConvert.height.show_units());
	}

	void set_tracker_tool_tip() {
		if (tracker_motion_value.isEnabled())
			tracker_motion_value.setToolTipText("How far the device must move before logging");
		else
			tracker_motion_value.setToolTipText("This device doesn't disable logging when stationary");
		if (tracker_interval_value.isEnabled())
			tracker_interval_value.setToolTipText("How often to report GPS position");
		else
			tracker_interval_value.setToolTipText("This device can't configure interval");
	}

	public void set_tracker_motion(int tracker_motion) {
		if (tracker_motion < 0) {
			tracker_motion_label.setVisible(false);
			tracker_motion_value.setVisible(false);
		} else {
			tracker_motion_label.setVisible(true);
			tracker_motion_value.setVisible(true);
			tracker_motion_value.setSelectedItem(AltosConvert.height.say(tracker_motion));
		}
	}

	public int tracker_motion() throws AltosConfigDataException {
		return (int) AltosConvert.height.parse(tracker_motion_value.getSelectedItem().toString());
	}

	public void set_tracker_interval(int tracker_interval) {
		if (tracker_interval< 0) {
			tracker_interval_label.setVisible(false);
			tracker_interval_value.setVisible(false);
		} else {
			tracker_interval_label.setVisible(true);
			tracker_interval_value.setVisible(true);
			tracker_interval_value.setSelectedItem(String.format("%d", tracker_interval));
		}
	}

	public int tracker_interval() throws AltosConfigDataException {
		return parse_int ("tracker interval", tracker_interval_value.getSelectedItem().toString(), false);
	}

	public void set_pyros(AltosPyro[] new_pyros) {
		pyros = new_pyros;
		pyro.setVisible(pyros != null);
		if (pyros != null && pyro_ui != null)
			pyro_ui.set_pyros(pyros);
	}

	public AltosPyro[] pyros() throws AltosConfigDataException {
		if (pyro_ui != null)
			pyros = pyro_ui.get_pyros();
		return pyros;
	}

	public void set_pyro_firing_time(double new_pyro_firing_time) {
		pyro_firing_time = new_pyro_firing_time;
		pyro.setVisible(pyro_firing_time >= 0);
		if (pyro_firing_time >= 0 && pyro_ui != null)
			pyro_ui.set_pyro_firing_time(pyro_firing_time);
	}

	public double pyro_firing_time() throws AltosConfigDataException {
		if (pyro_ui != null)
			pyro_firing_time = pyro_ui.get_pyro_firing_time();
		return pyro_firing_time;
	}

	public void set_aprs_interval(int new_aprs_interval) {
		String	s;

		if (new_aprs_interval <= 0)
			s = "Disabled";
		else
			s = Integer.toString(new_aprs_interval);
		aprs_interval_value.setSelectedItem(s);
		aprs_interval_value.setVisible(new_aprs_interval >= 0);
		set_aprs_interval_tool_tip();
	}

	public int aprs_interval() throws AltosConfigDataException {
		String	s = aprs_interval_value.getSelectedItem().toString();

		if (s.equals("Disabled"))
			return 0;
		return parse_int("aprs interval", s, false);
	}
}

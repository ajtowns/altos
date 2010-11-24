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
import java.util.concurrent.*;

public class AltosIgniteUI
	extends JDialog
	implements ActionListener
{
	AltosDevice	device;
	AltosIgnite	ignite;
	JFrame		owner;
	JLabel		label;
	JRadioButton	apogee;
	JLabel		apogee_status_label;
	JRadioButton	main;
	JLabel		main_status_label;
	JToggleButton	arm;
	JButton		fire;
	javax.swing.Timer	timer;

	int		apogee_status;
	int		main_status;

	final static int	timeout = 1 * 1000;

	int		time_remaining;
	boolean		timer_running;

	void set_arm_text() {
		if (arm.isSelected())
			arm.setText(String.format("%d", time_remaining));
		else
			arm.setText("Arm");
	}

	void start_timer() {
		time_remaining = 10;
		set_arm_text();
		timer_running = true;
	}

	void stop_timer() {
		time_remaining = 0;
		arm.setSelected(false);
		arm.setEnabled(false);
		fire.setEnabled(false);
		timer_running = false;
		set_arm_text();
	}

	void cancel () {
		apogee.setSelected(false);
		main.setSelected(false);
		fire.setEnabled(false);
		stop_timer();
	}

	void get_ignite_status() throws InterruptedException, TimeoutException {
		apogee_status = ignite.status(AltosIgnite.Apogee);
		main_status = ignite.status(AltosIgnite.Main);
	}

	void set_ignite_status() throws InterruptedException, TimeoutException {
		get_ignite_status();
		apogee_status_label.setText(String.format("\"%s\"", ignite.status_string(apogee_status)));
		main_status_label.setText(String.format("\"%s\"", ignite.status_string(main_status)));
	}

	void close() {
		timer.stop();
		setVisible(false);
		ignite.close();
	}

	void abort() {
		close();
		JOptionPane.showMessageDialog(owner,
					      String.format("Connection to \"%s\" failed",
							    device.toShortString()),
					      "Connection Failed",
					      JOptionPane.ERROR_MESSAGE);
	}

	void tick_timer() {
		if (timer_running) {
			--time_remaining;
			if (time_remaining <= 0)
				cancel();
			else
				set_arm_text();
		}
		try {
			set_ignite_status();
		} catch (InterruptedException ie) {
			abort();
		} catch (TimeoutException te) {
			abort();
		}
	}

	void fire() {
		if (arm.isEnabled() && arm.isSelected() && time_remaining > 0) {
			int	igniter = AltosIgnite.None;
			if (apogee.isSelected() && !main.isSelected())
				igniter = AltosIgnite.Apogee;
			else if (main.isSelected() && !apogee.isSelected())
				igniter = AltosIgnite.Main;
			ignite.fire(igniter);
			cancel();
		}
	}

	public void actionPerformed(ActionEvent e) {
		String cmd = e.getActionCommand();
		if (cmd.equals("apogee") || cmd.equals("main")) {
			stop_timer();
		}

		if (cmd.equals("apogee") && apogee.isSelected()) {
			main.setSelected(false);
			arm.setEnabled(true);
		}
		if (cmd.equals("main") && main.isSelected()) {
			apogee.setSelected(false);
			arm.setEnabled(true);
		}

		if (cmd.equals("arm")) {
			if (arm.isSelected()) {
				fire.setEnabled(true);
				start_timer();
			} else
				cancel();
		}
		if (cmd.equals("fire"))
			fire();
		if (cmd.equals("tick"))
			tick_timer();
		if (cmd.equals("close")) {
			close();
		}
	}

	/* A window listener to catch closing events and tell the config code */
	class ConfigListener extends WindowAdapter {
		AltosIgniteUI	ui;

		public ConfigListener(AltosIgniteUI this_ui) {
			ui = this_ui;
		}

		public void windowClosing(WindowEvent e) {
			ui.actionPerformed(new ActionEvent(e.getSource(),
							   ActionEvent.ACTION_PERFORMED,
							   "close"));
		}
	}

	private boolean open() {
		device = AltosDeviceDialog.show(owner, AltosDevice.product_any);
		if (device != null) {
			try {
				ignite = new AltosIgnite(device);
				return true;
			} catch (FileNotFoundException ee) {
				JOptionPane.showMessageDialog(owner,
							      String.format("Cannot open device \"%s\"",
									    device.toShortString()),
							      "Cannot open target device",
							      JOptionPane.ERROR_MESSAGE);
			} catch (AltosSerialInUseException si) {
				JOptionPane.showMessageDialog(owner,
							      String.format("Device \"%s\" already in use",
									    device.toShortString()),
							      "Device in use",
							      JOptionPane.ERROR_MESSAGE);
			} catch (IOException ee) {
				JOptionPane.showMessageDialog(owner,
							      device.toShortString(),
							      ee.getLocalizedMessage(),
							      JOptionPane.ERROR_MESSAGE);
			}
		}
		return false;
	}

	public AltosIgniteUI(JFrame in_owner) {

		owner = in_owner;
		apogee_status = AltosIgnite.Unknown;
		main_status = AltosIgnite.Unknown;

		if (!open())
			return;

		Container		pane = getContentPane();
		GridBagConstraints	c = new GridBagConstraints();
		Insets			i = new Insets(4,4,4,4);

		timer = new javax.swing.Timer(timeout, this);
		timer.setActionCommand("tick");
		timer_running = false;
		timer.restart();

		owner = in_owner;

		pane.setLayout(new GridBagLayout());

		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		c.weightx = 1;
		c.weighty = 1;

		c.gridx = 0;
		c.gridy = 0;
		c.gridwidth = 2;
		c.anchor = GridBagConstraints.CENTER;
		label = new JLabel ("Fire Igniter");
		pane.add(label, c);

		c.gridx = 0;
		c.gridy = 1;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.WEST;
		apogee = new JRadioButton ("Apogee");
		pane.add(apogee, c);
		apogee.addActionListener(this);
		apogee.setActionCommand("apogee");

		c.gridx = 1;
		c.gridy = 1;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.WEST;
		apogee_status_label = new JLabel();
		pane.add(apogee_status_label, c);

		c.gridx = 0;
		c.gridy = 2;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.WEST;
		main = new JRadioButton ("Main");
		pane.add(main, c);
		main.addActionListener(this);
		main.setActionCommand("main");

		c.gridx = 1;
		c.gridy = 2;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.WEST;
		main_status_label = new JLabel();
		pane.add(main_status_label, c);

		try {
			set_ignite_status();
		} catch (InterruptedException ie) {
			abort();
			return;
		} catch (TimeoutException te) {
			abort();
			return;
		}

		c.gridx = 0;
		c.gridy = 3;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.CENTER;
		arm = new JToggleButton ("Arm");
		pane.add(arm, c);
		arm.addActionListener(this);
		arm.setActionCommand("arm");
		arm.setEnabled(false);

		c.gridx = 1;
		c.gridy = 3;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.CENTER;
		fire = new JButton ("Fire");
		fire.setEnabled(false);
		pane.add(fire, c);
		fire.addActionListener(this);
		fire.setActionCommand("fire");

		pack();
		setLocationRelativeTo(owner);
		setVisible(true);

		addWindowListener(new ConfigListener(this));
	}
}
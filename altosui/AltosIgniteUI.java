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
import org.altusmetrum.AltosLib.*;

public class AltosIgniteUI
	extends AltosDialog
	implements ActionListener
{
	AltosDevice	device;
	JFrame		owner;
	JLabel		label;
	JRadioButton	apogee;
	JLabel		apogee_status_label;
	JRadioButton	main;
	JLabel		main_status_label;
	JToggleButton	arm;
	JButton		fire;
	javax.swing.Timer	timer;
	JButton		close;

	int		apogee_status;
	int		main_status;

	final static int	timeout = 1 * 1000;

	int		time_remaining;
	boolean		timer_running;

	LinkedBlockingQueue<String>	command_queue;

	class IgniteHandler implements Runnable {
		AltosIgnite	ignite;
		JFrame		owner;

		void send_exception(Exception e) {
			final Exception	f_e = e;
			Runnable r = new Runnable() {
					public void run() {
						ignite_exception(f_e);
					}
				};
			SwingUtilities.invokeLater(r);
		}

		public void run () {
			try {
				ignite = new AltosIgnite(device);
			} catch (Exception e) {
				send_exception(e);
				return;
			}
			ignite.set_frame(owner);

			for (;;) {
				Runnable	r;

				try {
					String		command = command_queue.take();
					String		reply = null;

					if (command.equals("get_status")) {
						apogee_status = ignite.status(AltosIgnite.Apogee);
						main_status = ignite.status(AltosIgnite.Main);
						reply = "status";
					} else if (command.equals("main")) {
						ignite.fire(AltosIgnite.Main);
						reply = "fired";
					} else if (command.equals("apogee")) {
						ignite.fire(AltosIgnite.Apogee);
						reply = "fired";
					} else if (command.equals("quit")) {
						ignite.close();
						break;
					} else {
						throw new ParseException(String.format("invalid command %s", command), 0);
					}
					final String f_reply = reply;
					r = new Runnable() {
							public void run() {
								ignite_reply(f_reply);
							}
						};
					SwingUtilities.invokeLater(r);
				} catch (Exception e) {
					send_exception(e);
				}
			}
		}

		public IgniteHandler(JFrame in_owner) {
			owner = in_owner;
		}
	}

	void ignite_exception(Exception e) {
		if (e instanceof FileNotFoundException) {
			JOptionPane.showMessageDialog(owner,
						      ((FileNotFoundException) e).getMessage(),
						      "Cannot open target device",
						      JOptionPane.ERROR_MESSAGE);
		} else if (e instanceof AltosSerialInUseException) {
			JOptionPane.showMessageDialog(owner,
						      String.format("Device \"%s\" already in use",
								    device.toShortString()),
						      "Device in use",
						      JOptionPane.ERROR_MESSAGE);
		} else if (e instanceof IOException) {
			IOException ee = (IOException) e;
			JOptionPane.showMessageDialog(owner,
						      device.toShortString(),
						      ee.getLocalizedMessage(),
						      JOptionPane.ERROR_MESSAGE);
		} else {
			JOptionPane.showMessageDialog(owner,
						      String.format("Connection to \"%s\" failed",
								    device.toShortString()),
						      "Connection Failed",
						      JOptionPane.ERROR_MESSAGE);
		}
		close();
	}

	void ignite_reply(String reply) {
		if (reply.equals("status")) {
			set_ignite_status();
		} else if (reply.equals("fired")) {
			fired();
		}
	}

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

	void send_command(String command) {
		try {
			command_queue.put(command);
		} catch (Exception ex) {
			ignite_exception(ex);
		}
	}

	boolean	getting_status = false;

	boolean	visible = false;
	void set_ignite_status() {
		getting_status = false;
		apogee_status_label.setText(String.format("\"%s\"", AltosIgnite.status_string(apogee_status)));
		main_status_label.setText(String.format("\"%s\"", AltosIgnite.status_string(main_status)));
		if (!visible) {
			visible = true;
			setVisible(true);
		}
	}

	void poll_ignite_status() {
		if (!getting_status) {
			getting_status = true;
			send_command("get_status");
		}
	}

	boolean	firing = false;

	void start_fire(String which) {
		if (!firing) {
			firing = true;
			send_command(which);
		}
	}

	void fired() {
		firing = false;
		cancel();
	}

	void close() {
		send_command("quit");
		timer.stop();
		setVisible(false);
		dispose();
	}

	void tick_timer() {
		if (timer_running) {
			--time_remaining;
			if (time_remaining <= 0)
				cancel();
			else
				set_arm_text();
		}
		poll_ignite_status();
	}

	void fire() {
		if (arm.isEnabled() && arm.isSelected() && time_remaining > 0) {
			String	igniter = "none";
			if (apogee.isSelected() && !main.isSelected())
				igniter = "apogee";
			else if (main.isSelected() && !apogee.isSelected())
				igniter = "main";
			send_command(igniter);
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
		command_queue = new LinkedBlockingQueue<String>();

		device = AltosDeviceDialog.show(owner, Altos.product_any);
		if (device != null) {
				IgniteHandler	handler = new IgniteHandler(owner);
				Thread		t = new Thread(handler);
				t.start();
				return true;
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
		c.weightx = 0;
		c.weighty = 0;

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

		c.gridx = 0;
		c.gridy = 4;
		c.gridwidth = 2;
		c.anchor = GridBagConstraints.CENTER;
		close = new JButton ("Close");
		pane.add(close, c);
		close.addActionListener(this);
		close.setActionCommand("close");
			
		pack();
		setLocationRelativeTo(owner);

		addWindowListener(new ConfigListener(this));
	}
}
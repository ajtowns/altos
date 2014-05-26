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
import java.io.*;
import java.text.*;
import java.util.*;
import java.util.concurrent.*;
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

public class AltosIgniteUI
	extends AltosUIDialog
	implements ActionListener
{
	AltosDevice	device;
	JFrame		owner;
	JLabel		label;
	JToggleButton	arm;
	JButton		fire;
	javax.swing.Timer	timer;
	JButton		close;
	ButtonGroup	group;
	Boolean		opened;

	int		npyro;

	final static int	timeout = 1 * 1000;

	int		time_remaining;
	boolean		timer_running;

	LinkedBlockingQueue<String>	command_queue;

	LinkedBlockingQueue<String>	reply_queue;

	class Igniter {
		JRadioButton	button;
		JLabel		status_label;
		String		name;
		int		status;

		void set_status (int status) {
			this.status = status;
			status_label.setText(String.format("\"%s\"", AltosIgnite.status_string(status)));
		}

		Igniter(AltosIgniteUI ui, String label, String name, int y) {
			Container		pane = getContentPane();
			GridBagConstraints	c = new GridBagConstraints();
			Insets			i = new Insets(4,4,4,4);

			this.name = name;
			this.status = AltosIgnite.Unknown;

			c.gridx = 0;
			c.gridy = y;
			c.gridwidth = 1;
			c.anchor = GridBagConstraints.WEST;
			button = new JRadioButton (label);
			pane.add(button, c);
			button.addActionListener(ui);
			button.setActionCommand(name);
			group.add(button);

			c.gridx = 1;
			c.gridy = y;
			c.gridwidth = 1;
			c.anchor = GridBagConstraints.WEST;
			status_label = new JLabel("plenty of text");
			pane.add(status_label, c);

			status = AltosIgnite.Unknown;
		}
	}

	Igniter	igniters[];

	void set_status(String _name, int _status) {

		final String name = _name;
		final int status = _status;
		Runnable r = new Runnable() {
				public void run() {
					for (int p = 0; p < igniters.length; p++)
						if (name.equals(igniters[p].name))
							igniters[p].set_status(status);
				}
			};
		SwingUtilities.invokeLater(r);
	}

	class IgniteHandler implements Runnable {
		AltosIgnite	ignite;
		JFrame		owner;
		AltosLink	link;

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
				ignite = new AltosIgnite(link,
							 !device.matchProduct(Altos.product_altimeter));

			} catch (Exception e) {
				send_exception(e);
				return;
			}

			for (;;) {
				Runnable	r;

				try {
					String		command = command_queue.take();
					String		reply = null;

					if (command.equals("get_status")) {
						HashMap<String,Integer> status_map = ignite.status();

						for (int p = 0; p < igniters.length; p++) {
							Integer i = status_map.get(igniters[p].name);
							if (i != null)
								set_status(igniters[p].name, i);
						}
						reply = "status";
					} else if (command.equals("get_npyro")) {
						put_reply(String.format("%d", ignite.npyro()));
						continue;
					} else if (command.equals("quit")) {
						ignite.close();
						break;
					} else {
						ignite.fire(command);
						reply = "fired";
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

		public IgniteHandler(JFrame in_owner, AltosLink in_link) {
			owner = in_owner;
			link = in_link;
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
		fire.setEnabled(false);
		timer_running = false;
		arm.setSelected(false);
		arm.setEnabled(false);
		set_arm_text();
	}

	void cancel () {
		group.clearSelection();
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

	void put_reply(String reply) {
		try {
			reply_queue.put(reply);
		} catch (Exception ex) {
			ignite_exception(ex);
		}
	}

	String get_reply() {
		String reply = "";
		try {
			reply = reply_queue.take();
		} catch (Exception ex) {
			ignite_exception(ex);
		}
		return reply;
	}

	boolean	getting_status = false;

	boolean	visible = false;

	void set_ignite_status() {
		getting_status = false;
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

	int get_npyro() {
		send_command("get_npyro");
		String reply = get_reply();
		return Integer.parseInt(reply);
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
		if (opened) {
			send_command("quit");
			timer.stop();
		}
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

			for (int p = 0; p < igniters.length; p++)
				if (igniters[p].button.isSelected()) {
					igniter = igniters[p].name;
					break;
				}
			send_command(igniter);
			cancel();
		}
	}

	public void actionPerformed(ActionEvent e) {
		String cmd = e.getActionCommand();

		for (int p = 0; p < igniters.length; p++)
			if (cmd.equals(igniters[p].name)) {
				stop_timer();
				arm.setEnabled(true);
				break;
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
		if (cmd.equals("close"))
			close();
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
		reply_queue = new LinkedBlockingQueue<String>();

		opened = false;
		device = AltosDeviceUIDialog.show(owner, Altos.product_any);
		if (device != null) {
			try {
				AltosSerial	serial = new AltosSerial(device);
				serial.set_frame(owner);
				IgniteHandler	handler = new IgniteHandler(owner, serial);
				Thread		t = new Thread(handler);
				t.start();
				opened = true;
				return true;
			} catch (Exception ex) {
				ignite_exception(ex);
			}
		}
		return false;
	}

	public AltosIgniteUI(JFrame in_owner) {

		owner = in_owner;

		if (!open())
			return;

		group = new ButtonGroup();

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

		int y = 0;

		c.gridx = 0;
		c.gridy = y;
		c.gridwidth = 2;
		c.anchor = GridBagConstraints.CENTER;
		label = new JLabel ("Fire Igniter");
		pane.add(label, c);

		y++;

		int npyro = get_npyro();

		igniters = new Igniter[2 + npyro];

		igniters[0] = new Igniter(this, "Apogee", AltosIgnite.Apogee, y++);
		igniters[1] = new Igniter(this, "Main", AltosIgnite.Main, y++);

		for (int p = 0; p < npyro; p++) {
			String	name = String.format("%d", p);
			String	label = String.format("%c", 'A' + p);
			igniters[2+p] = new Igniter(this, label, name, y++);
		}

		c.gridx = 0;
		c.gridy = y;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.CENTER;
		arm = new JToggleButton ("Arm");
		pane.add(arm, c);
		arm.addActionListener(this);
		arm.setActionCommand("arm");
		arm.setEnabled(false);

		c.gridx = 1;
		c.gridy = y;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.CENTER;
		fire = new JButton ("Fire");
		fire.setEnabled(false);
		pane.add(fire, c);
		fire.addActionListener(this);
		fire.setActionCommand("fire");

		y++;

		c.gridx = 0;
		c.gridy = y;
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
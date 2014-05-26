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
import java.util.concurrent.*;
import org.altusmetrum.altosuilib_2.*;

class FireButton extends JButton {
	protected void processMouseEvent(MouseEvent e) {
		super.processMouseEvent(e);
		switch (e.getID()) {
		case MouseEvent.MOUSE_PRESSED:
			if (actionListener != null)
				actionListener.actionPerformed(new ActionEvent(this, e.getID(), "fire_down"));
			break;
		case MouseEvent.MOUSE_RELEASED:
			if (actionListener != null)
				actionListener.actionPerformed(new ActionEvent(this, e.getID(), "fire_up"));
			break;
		}
	}

	public FireButton(String s) {
		super(s);
	}
}

public class AltosLaunchUI
	extends AltosUIDialog
	implements ActionListener
{
	AltosDevice	device;
	JFrame		owner;
	JLabel		label;

	int		radio_channel;
	JLabel		radio_channel_label;
	JTextField	radio_channel_text;

	int		launcher_serial;
	JLabel		launcher_serial_label;
	JTextField	launcher_serial_text;

	int		launcher_channel;
	JLabel		launcher_channel_label;
	JTextField	launcher_channel_text;

	JLabel		armed_label;
	JLabel		armed_status_label;
	JLabel		igniter;
	JLabel		igniter_status_label;
	JToggleButton	arm;
	FireButton	fire;
	javax.swing.Timer	arm_timer;
	javax.swing.Timer	fire_timer;

	boolean		firing;
	boolean		armed;
	int		armed_status;
	int		igniter_status;
	int		rssi;

	final static int	arm_timeout = 1 * 1000;
	final static int	fire_timeout = 250;

	int		armed_count;

	LinkedBlockingQueue<String>	command_queue;

	class LaunchHandler implements Runnable {
		AltosLaunch	launch;
		JFrame		owner;

		void send_exception(Exception e) {
			final Exception	f_e = e;
			Runnable r = new Runnable() {
					public void run() {
						launch_exception(f_e);
					}
				};
			SwingUtilities.invokeLater(r);
		}

		public void run () {
			try {
				launch = new AltosLaunch(device);
			} catch (Exception e) {
				send_exception(e);
				return;
			}
			launch.set_frame(owner);
			launch.set_remote(launcher_serial, launcher_channel);

			for (;;) {
				Runnable	r;

				try {
					String		command = command_queue.take();
					String		reply = null;

					if (command.equals("get_status")) {
						launch.status();
						reply = "status";
						armed_status = launch.armed;
						igniter_status = launch.igniter;
						rssi = launch.rssi;
					} else if (command.equals("set_remote")) {
						launch.set_remote(launcher_serial, launcher_channel);
						reply = "remote set";
					} else if (command.equals("arm")) {
						launch.arm();
						reply = "armed";
					} else if (command.equals("fire")) {
						launch.fire();
						reply = "fired";
					} else if (command.equals("quit")) {
						launch.close();
						break;
					} else {
						throw new ParseException(String.format("invalid command %s", command), 0);
					}
					final String f_reply = reply;
					r = new Runnable() {
							public void run() {
								launch_reply(f_reply);
							}
						};
					SwingUtilities.invokeLater(r);
				} catch (Exception e) {
					send_exception(e);
				}
			}
		}

		public LaunchHandler(JFrame in_owner) {
			owner = in_owner;
		}
	}

	void launch_exception(Exception e) {
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

	void launch_reply(String reply) {
		if (reply == null)
			return;
		if (reply.equals("remote set"))
			poll_launch_status();
		if (reply.equals("status")) {
			set_launch_status();
		}
	}

	void set_arm_text() {
		if (arm.isSelected())
			arm.setText(String.format("%d", armed_count));
		else
			arm.setText("Arm");
	}

	void start_arm_timer() {
		armed_count = 30;
		set_arm_text();
	}

	void stop_arm_timer() {
		armed_count = 0;
		armed = false;
		arm.setSelected(false);
		fire.setEnabled(false);
		set_arm_text();
	}

	void cancel () {
		fire.setEnabled(false);
		firing = false;
		stop_arm_timer();
	}

	void send_command(String command) {
		try {
			command_queue.put(command);
		} catch (Exception ex) {
			launch_exception(ex);
		}
	}

	boolean	getting_status = false;

	void set_launch_status() {
		getting_status = false;
		armed_status_label.setText(String.format("\"%s\"", AltosLaunch.status_string(armed_status)));
		igniter_status_label.setText(String.format("\"%s\"", AltosLaunch.status_string(igniter_status)));
	}

	void poll_launch_status() {
		if (!getting_status && !firing && !armed) {
			getting_status = true;
			send_command("get_status");
		}
	}

	void fired() {
		firing = false;
		cancel();
	}

	void close() {
		send_command("quit");
		arm_timer.stop();
		setVisible(false);
		dispose();
	}

	void tick_arm_timer() {
		if (armed_count > 0) {
			--armed_count;
			if (armed_count <= 0) {
				armed_count = 0;
				cancel();
			} else {
				if (!firing) {
					send_command("arm");
					set_arm_text();
				}
			}
		}
		poll_launch_status();
	}

	void arm() {
		if (arm.isSelected()) {
			fire.setEnabled(true);
			start_arm_timer();
			if (!firing)
				send_command("arm");
			armed = true;
		} else
			cancel();
	}

	void fire_more() {
		if (firing)
			send_command("fire");
	}

	void fire_down() {
		if (arm.isEnabled() && arm.isSelected() && armed_count > 0) {
			firing = true;
			fire_more();
			fire_timer.restart();
		}
	}

	void fire_up() {
		firing = false;
		fire_timer.stop();
	}

	void set_radio() {
		try {
			radio_channel = Integer.parseInt(radio_channel_text.getText());
		} catch (NumberFormatException ne) {
			radio_channel_text.setText(String.format("%d", radio_channel));
		}
	}

	void set_serial() {
		try {
			launcher_serial = Integer.parseInt(launcher_serial_text.getText());
			AltosUIPreferences.set_launcher_serial(launcher_serial);
			send_command("set_remote");
		} catch (NumberFormatException ne) {
			launcher_serial_text.setText(String.format("%d", launcher_serial));
		}
	}

	void set_channel() {
		try {
			launcher_channel = Integer.parseInt(launcher_channel_text.getText());
			AltosUIPreferences.set_launcher_serial(launcher_channel);
			send_command("set_remote");
		} catch (NumberFormatException ne) {
			launcher_channel_text.setText(String.format("%d", launcher_channel));
		}
	}

	public void actionPerformed(ActionEvent e) {
		String cmd = e.getActionCommand();
		if (cmd.equals("armed") || cmd.equals("igniter")) {
			stop_arm_timer();
		}

		if (cmd.equals("arm"))
			arm();
		if (cmd.equals("tick_arm"))
			tick_arm_timer();
		if (cmd.equals("close"))
			close();
		if (cmd.equals("fire_down"))
			fire_down();
		if (cmd.equals("fire_up"))
			fire_up();
		if (cmd.equals("tick_fire"))
			fire_more();
		if (cmd.equals("new_serial"))
			set_serial();
		if (cmd.equals("new_channel"))
			set_channel();
	}

	/* A window listener to catch closing events and tell the config code */
	class ConfigListener extends WindowAdapter {
		AltosLaunchUI	ui;

		public ConfigListener(AltosLaunchUI this_ui) {
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

		device = AltosDeviceUIDialog.show(owner, Altos.product_any);
		if (device != null) {
				LaunchHandler	handler = new LaunchHandler(owner);
				Thread		t = new Thread(handler);
				t.start();
				return true;
		}
		return false;
	}

	public AltosLaunchUI(JFrame in_owner) {

		launcher_channel = AltosUIPreferences.launcher_channel();
		launcher_serial = AltosUIPreferences.launcher_serial();
		owner = in_owner;
		armed_status = AltosLaunch.Unknown;
		igniter_status = AltosLaunch.Unknown;

		if (!open())
			return;

		Container		pane = getContentPane();
		GridBagConstraints	c = new GridBagConstraints();
		Insets			i = new Insets(4,4,4,4);

		arm_timer = new javax.swing.Timer(arm_timeout, this);
		arm_timer.setActionCommand("tick_arm");
		arm_timer.restart();

		fire_timer = new javax.swing.Timer(fire_timeout, this);
		fire_timer.setActionCommand("tick_fire");

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
		label = new JLabel ("Launch Controller");
		pane.add(label, c);

		c.gridx = 0;
		c.gridy = 1;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.WEST;
		launcher_serial_label = new JLabel("Launcher Serial");
		pane.add(launcher_serial_label, c);

		c.gridx = 1;
		c.gridy = 1;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.WEST;
		launcher_serial_text = new JTextField(7);
		launcher_serial_text.setText(String.format("%d", launcher_serial));
		launcher_serial_text.setActionCommand("new_serial");
		launcher_serial_text.addActionListener(this);
		pane.add(launcher_serial_text, c);

		c.gridx = 0;
		c.gridy = 2;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.WEST;
		launcher_channel_label = new JLabel("Launcher Channel");
		pane.add(launcher_channel_label, c);

		c.gridx = 1;
		c.gridy = 2;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.WEST;
		launcher_channel_text = new JTextField(7);
		launcher_channel_text.setText(String.format("%d", launcher_channel));
		launcher_channel_text.setActionCommand("new_channel");
		launcher_channel_text.addActionListener(this);
		pane.add(launcher_channel_text, c);

		c.gridx = 0;
		c.gridy = 3;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.WEST;
		armed_label = new JLabel ("Armed");
		pane.add(armed_label, c);

		c.gridx = 1;
		c.gridy = 3;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.WEST;
		armed_status_label = new JLabel();
		pane.add(armed_status_label, c);

		c.gridx = 0;
		c.gridy = 4;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.WEST;
		igniter = new JLabel ("Igniter");
		pane.add(igniter, c);

		c.gridx = 1;
		c.gridy = 4;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.WEST;
		igniter_status_label = new JLabel();
		pane.add(igniter_status_label, c);

		c.gridx = 0;
		c.gridy = 5;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.CENTER;
		arm = new JToggleButton ("Arm");
		pane.add(arm, c);
		arm.addActionListener(this);
		arm.setActionCommand("arm");
		arm.setEnabled(true);

		c.gridx = 1;
		c.gridy = 5;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.CENTER;
		fire = new FireButton ("Fire");
		fire.setEnabled(false);
		pane.add(fire, c);
		fire.addActionListener(this);
		fire.setActionCommand("fire");

		pack();
		setLocationRelativeTo(owner);

		addWindowListener(new ConfigListener(this));

		setVisible(true);
	}
}
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

public class AltosIgniteUI
	extends JDialog
	implements ActionListener
{
	JFrame		owner;
	JLabel		label;
	JRadioButton	apogee;
	JRadioButton	main;
	JToggleButton	arm;
	JButton		fire;
	javax.swing.Timer	timer;

	final static int	timeout = 1 * 1000;

	int		time_remaining;

	void set_arm_text() {
		if (arm.isSelected())
			arm.setText(String.format("%d", time_remaining));
		else
			arm.setText("Arm");
	}

	void start_timer() {
		time_remaining = 10;
		set_arm_text();
		timer.restart();
	}

	void stop_timer() {
		time_remaining = 0;
		arm.setSelected(false);
		arm.setEnabled(false);
		fire.setEnabled(false);
		timer.stop();
		set_arm_text();
	}

	void cancel () {
		apogee.setSelected(false);
		main.setSelected(false);
		fire.setEnabled(false);
		stop_timer();
	}

	void tick_timer() {
		--time_remaining;
		if (time_remaining <= 0)
			cancel();
		else
			set_arm_text();
	}

	void fire() {
		if (arm.isEnabled() && arm.isSelected() && time_remaining > 0) {
			int	igniter = AltosIgnite.None;
			if (apogee.isSelected() && !main.isSelected())
				igniter = AltosIgnite.Apogee;
			else if (main.isSelected() && !apogee.isSelected())
				igniter = AltosIgnite.Main;
			System.out.printf ("fire %d\n", igniter);
			cancel();
		}
	}

	public void actionPerformed(ActionEvent e) {
		String cmd = e.getActionCommand();
		if (cmd.equals("apogee") || cmd.equals("main")) {
			stop_timer();
			arm.setEnabled(true);
		}

		if (cmd.equals("apogee") && apogee.isSelected())
			main.setSelected(false);
		if (cmd.equals("main") && main.isSelected())
			apogee.setSelected(false);

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
	}

	public AltosIgniteUI(JFrame in_owner) {
		Container		pane = getContentPane();
		GridBagConstraints	c = new GridBagConstraints();
		Insets			i = new Insets(4,4,4,4);

		timer = new javax.swing.Timer(timeout, this);
		timer.setActionCommand("tick");

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
		label = new JLabel ("Fire Igniter");
		pane.add(label, c);

		c.gridx = 0;
		c.gridy = 1;
		c.gridwidth = 1;
		apogee = new JRadioButton ("Apogee");
		pane.add(apogee, c);
		apogee.addActionListener(this);
		apogee.setActionCommand("apogee");

		c.gridx = 1;
		c.gridy = 1;
		c.gridwidth = 1;
		main = new JRadioButton ("Main");
		pane.add(main, c);
		main.addActionListener(this);
		main.setActionCommand("main");

		c.gridx = 0;
		c.gridy = 2;
		c.gridwidth = 1;
		arm = new JToggleButton ("Arm");
		pane.add(arm, c);
		arm.addActionListener(this);
		arm.setActionCommand("arm");
		arm.setEnabled(false);

		c.gridx = 1;
		c.gridy = 2;
		c.gridwidth = 1;
		fire = new JButton ("Fire");
		fire.setEnabled(false);
		pane.add(fire, c);
		fire.addActionListener(this);
		fire.setActionCommand("fire");

		pack();
		setLocationRelativeTo(owner);
		setVisible(true);
	}
}
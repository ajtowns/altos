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

package org.altusmetrum.altosuilib_2;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import org.altusmetrum.altoslib_4.*;

public class AltosRomconfigUI
	extends AltosUIDialog
	implements ActionListener
{
	Container	pane;
	Box		box;
	JLabel		serial_label;
	JLabel		radio_calibration_label;

	JFrame		owner;
	JTextField	serial_value;
	JTextField	radio_calibration_value;

	JButton		ok;
	JButton		cancel;

	/* Build the UI using a grid bag */
	public AltosRomconfigUI(JFrame in_owner) {
		super (in_owner, "Configure TeleMetrum Rom Values", true);

		owner = in_owner;
		GridBagConstraints c;

		Insets il = new Insets(4,4,4,4);
		Insets ir = new Insets(4,4,4,4);

		pane = getContentPane();
		pane.setLayout(new GridBagLayout());

		/* Serial */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 0;
		c.gridwidth = 3;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		serial_label = new JLabel("Serial:");
		pane.add(serial_label, c);

		c = new GridBagConstraints();
		c.gridx = 3; c.gridy = 0;
		c.gridwidth = 3;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		serial_value = new JTextField("00000000");
		pane.add(serial_value, c);

		/* Radio calibration value */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 1;
		c.gridwidth = 3;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		radio_calibration_label = new JLabel("Radio Calibration:");
		pane.add(radio_calibration_label, c);

		c = new GridBagConstraints();
		c.gridx = 3; c.gridy = 1;
		c.gridwidth = 3;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		radio_calibration_value = new JTextField("00000000");
		pane.add(radio_calibration_value, c);

		/* Buttons */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 2;
		c.gridwidth = 3;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = il;
		ok = new JButton("OK");
		pane.add(ok, c);
		ok.addActionListener(this);
		ok.setActionCommand("ok");

		c = new GridBagConstraints();
		c.gridx = 3; c.gridy = 2;
		c.gridwidth = 3;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = il;
		cancel = new JButton("Cancel");
		pane.add(cancel, c);
		cancel.addActionListener(this);
		cancel.setActionCommand("cancel");

		pack();
		setLocationRelativeTo(owner);
	}

	public AltosRomconfigUI(JFrame frame, AltosRomconfig config) {
		this(frame);
		set(config);
	}

	boolean	selected;

	/* Listen for events from our buttons */
	public void actionPerformed(ActionEvent e) {
		String	cmd = e.getActionCommand();

		if (cmd.equals("ok")) {
			AltosRomconfig	romconfig = romconfig();
			if (romconfig == null || !romconfig.valid()) {
				JOptionPane.showMessageDialog(this,
							      "Invalid serial number or radio calibration value",
							      "Invalid rom configuration",
							      JOptionPane.ERROR_MESSAGE);
				return;
			}
			selected = true;
		}
		setVisible(false);
	}

	int serial() {
		return Integer.parseInt(serial_value.getText());
	}

	void set_serial(int serial) {
		serial_value.setText(String.format("%d", serial));
	}

	int radio_calibration() {
		return Integer.parseInt(radio_calibration_value.getText());
	}

	void set_radio_calibration(int calibration) {
		radio_calibration_value.setText(String.format("%d", calibration));
	}

	public void set(AltosRomconfig config) {
		if (config != null && config.valid()) {
			set_serial(config.serial_number);
			set_radio_calibration(config.radio_calibration);
		}
	}

	AltosRomconfig romconfig() {
		try {
			return new AltosRomconfig(serial(), radio_calibration());
		} catch (NumberFormatException ne) {
			return null;
		}
	}

	public AltosRomconfig showDialog() {
		setVisible(true);
		if (selected)
			return romconfig();
		return null;
	}

	public static AltosRomconfig show(JFrame frame, AltosRomconfig config) {
		AltosRomconfigUI ui = new AltosRomconfigUI(frame, config);
		return ui.showDialog();
	}
}

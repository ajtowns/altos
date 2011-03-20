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

public class AltosConfigureUI
	extends JDialog
	implements DocumentListener
{
	JFrame		owner;
	AltosVoice	voice;
	Container	pane;

	JRadioButton	enable_voice;
	JButton		test_voice;
	JButton		close;

	JButton		configure_log;
	JTextField	log_directory;

	JLabel		callsign_label;
	JTextField	callsign_value;

	JRadioButton	serial_debug;

	/* DocumentListener interface methods */
	public void changedUpdate(DocumentEvent e) {
		AltosPreferences.set_callsign(callsign_value.getText());
	}

	public void insertUpdate(DocumentEvent e) {
		changedUpdate(e);
	}

	public void removeUpdate(DocumentEvent e) {
		changedUpdate(e);
	}

	public AltosConfigureUI(JFrame in_owner, AltosVoice in_voice) {
		super(in_owner, "Configure AltosUI", false);

		GridBagConstraints	c;

		Insets insets = new Insets(4, 4, 4, 4);

		owner = in_owner;
		voice = in_voice;
		pane = getContentPane();
		pane.setLayout(new GridBagLayout());

		c = new GridBagConstraints();
		c.insets = insets;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;

		/* Nice label at the top */
		c.gridx = 0;
		c.gridy = 0;
		c.gridwidth = 3;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		pane.add(new JLabel ("Configure AltOS UI"), c);

		c.gridx = 0;
		c.gridy = 1;
		c.gridwidth = 3;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		pane.add(new JLabel (String.format("AltOS version %s", AltosVersion.version)), c);

		/* Voice settings */
		c.gridx = 0;
		c.gridy = 2;
		c.gridwidth = 1;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		pane.add(new JLabel("Voice"), c);

		enable_voice = new JRadioButton("Enable", AltosPreferences.voice());
		enable_voice.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					JRadioButton item = (JRadioButton) e.getSource();
					boolean enabled = item.isSelected();
					AltosPreferences.set_voice(enabled);
					if (enabled)
						voice.speak_always("Enable voice.");
					else
						voice.speak_always("Disable voice.");
				}
			});
		c.gridx = 1;
		c.gridy = 2;
		c.gridwidth = 1;
		c.weightx = 1;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		pane.add(enable_voice, c);

		c.gridx = 2;
		c.gridy = 2;
		c.gridwidth = 1;
		c.weightx = 1;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.EAST;
		test_voice = new JButton("Test Voice");
		test_voice.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					voice.speak("That's one small step for man; one giant leap for mankind.");
				}
			});
		pane.add(test_voice, c);

		/* Log directory settings */
		c.gridx = 0;
		c.gridy = 3;
		c.gridwidth = 1;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		pane.add(new JLabel("Log Directory"), c);

		configure_log = new JButton(AltosPreferences.logdir().getPath());
		configure_log.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					AltosPreferences.ConfigureLog();
					configure_log.setText(AltosPreferences.logdir().getPath());
				}
			});
		c.gridx = 1;
		c.gridy = 3;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.BOTH;
		c.anchor = GridBagConstraints.WEST;
		pane.add(configure_log, c);

		/* Callsign setting */
		c.gridx = 0;
		c.gridy = 4;
		c.gridwidth = 1;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		pane.add(new JLabel("Callsign"), c);

		callsign_value = new JTextField(AltosPreferences.callsign());
		callsign_value.getDocument().addDocumentListener(this);
		c.gridx = 1;
		c.gridy = 4;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.BOTH;
		c.anchor = GridBagConstraints.WEST;
		pane.add(callsign_value, c);

		/* Serial debug setting */
		c.gridx = 0;
		c.gridy = 5;
		c.gridwidth = 1;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		pane.add(new JLabel("Serial Debug"), c);

		serial_debug = new JRadioButton("Enable", AltosPreferences.serial_debug());
		serial_debug.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					JRadioButton item = (JRadioButton) e.getSource();
					boolean enabled = item.isSelected();
					AltosPreferences.set_serial_debug(enabled);
				}
			});

		c.gridx = 1;
		c.gridy = 5;
		c.gridwidth = 3;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		pane.add(serial_debug, c);

		/* And a close button at the bottom */
		close = new JButton("Close");
		close.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					setVisible(false);
				}
			});
		c.gridx = 0;
		c.gridy = 6;
		c.gridwidth = 3;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		pane.add(close, c);

		pack();
		setLocationRelativeTo(owner);
		setVisible(true);
	}
}

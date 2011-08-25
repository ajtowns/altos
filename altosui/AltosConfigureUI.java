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

	JLabel		font_size_label;
	JComboBox	font_size_value;

	JRadioButton	serial_debug;

// BLUETOOTH
//	JButton		manage_bluetooth;
	JButton		manage_frequencies;

	final static String[] font_size_names = { "Small", "Medium", "Large" };

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

		int row = 0;

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
		c.gridy = row++;
		c.gridwidth = 3;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		pane.add(new JLabel ("Configure AltOS UI"), c);

		c.gridx = 0;
		c.gridy = row++;
		c.gridwidth = 3;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		pane.add(new JLabel (String.format("AltOS version %s", AltosVersion.version)), c);

		/* Voice settings */
		c.gridx = 0;
		c.gridy = row;
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
		c.gridy = row;
		c.gridwidth = 1;
		c.weightx = 1;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		pane.add(enable_voice, c);
		enable_voice.setToolTipText("Enable/Disable all audio in-flight announcements");

		c.gridx = 2;
		c.gridy = row++;
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
		test_voice.setToolTipText("Play a stock audio clip to check volume");

		/* Log directory settings */
		c.gridx = 0;
		c.gridy = row;
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
		c.gridy = row++;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.BOTH;
		c.anchor = GridBagConstraints.WEST;
		pane.add(configure_log, c);
		configure_log.setToolTipText("Which directory flight logs are stored in");

		/* Callsign setting */
		c.gridx = 0;
		c.gridy = row;
		c.gridwidth = 1;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		pane.add(new JLabel("Callsign"), c);

		callsign_value = new JTextField(AltosPreferences.callsign());
		callsign_value.getDocument().addDocumentListener(this);
		c.gridx = 1;
		c.gridy = row++;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.BOTH;
		c.anchor = GridBagConstraints.WEST;
		pane.add(callsign_value, c);
		callsign_value.setToolTipText("Callsign sent in packet mode");

		/* Font size setting */
		c.gridx = 0;
		c.gridy = row;
		c.gridwidth = 1;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		pane.add(new JLabel("Font size"), c);

		font_size_value = new JComboBox(font_size_names);
		int font_size = AltosPreferences.font_size();
		font_size_value.setSelectedIndex(font_size - Altos.font_size_small);
		font_size_value.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					int	size = font_size_value.getSelectedIndex() + Altos.font_size_small;

					AltosPreferences.set_font_size(size);
				}
			});
		c.gridx = 1;
		c.gridy = row++;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.BOTH;
		c.anchor = GridBagConstraints.WEST;
		pane.add(font_size_value, c);
		font_size_value.setToolTipText("Font size used in telemetry window");

		/* Serial debug setting */
		c.gridx = 0;
		c.gridy = row;
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
		serial_debug.setToolTipText("Enable/Disable USB I/O getting sent to the console");

		c.gridx = 1;
		c.gridy = row++;
		c.gridwidth = 3;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		pane.add(serial_debug, c);

// BLUETOOTH
//		manage_bluetooth = new JButton("Manage Bluetooth");
//		manage_bluetooth.addActionListener(new ActionListener() {
//				public void actionPerformed(ActionEvent e) {
//					AltosBTManage.show(owner, Altos.bt_known);
//				}
//			});
//		c.gridx = 0;
//		c.gridy = row++;
//		c.gridwidth = 2;
//		c.fill = GridBagConstraints.NONE;
//		c.anchor = GridBagConstraints.WEST;
//		pane.add(manage_bluetooth, c);

		manage_frequencies = new JButton("Manage Frequencies");
		manage_frequencies.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					AltosConfigFreqUI.show(owner);
				}
			});
		manage_frequencies.setToolTipText("Configure which values are shown in frequency menus");
// BLUETOOTH
//		c.gridx = 2;
		c.gridx = 1;
		c.gridy = row++;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		pane.add(manage_frequencies, c);

		/* And a close button at the bottom */
		close = new JButton("Close");
		close.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					setVisible(false);
				}
			});
		c.gridx = 0;
		c.gridy = row++;
		c.gridwidth = 3;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		pane.add(close, c);

		pack();
		setLocationRelativeTo(owner);
		setVisible(true);
	}
}

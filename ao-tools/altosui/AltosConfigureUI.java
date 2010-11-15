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
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.util.concurrent.LinkedBlockingQueue;

public class AltosConfigureUI extends JDialog {
	JFrame		owner;
	AltosVoice	voice;
	Container	pane;

	JRadioButton	enable_voice;
	JButton		test_voice;
	JButton		close;

	JButton		configure_log;
	JTextField	log_directory;

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
		c.anchor = GridBagConstraints.CENTER;

		/* Enable Voice */
		c.gridx = 0;
		c.gridy = 0;
		enable_voice = new JRadioButton("Enable Voice", AltosPreferences.voice());
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
		pane.add(enable_voice, c);
		c.gridx = 1;
		c.gridy = 0;
		test_voice = new JButton("Test Voice");
		test_voice.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					voice.speak("That's one small step for man; one giant leap for mankind.");
				}
			});
		pane.add(test_voice, c);

		close = new JButton("Close");
		close.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					setVisible(false);
				}
			});
		c.gridx = 0;
		c.gridy = 3;
		c.gridwidth = 2;
		pane.add(close, c);

		configure_log = new JButton("Configure Log");
		configure_log.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					AltosPreferences.ConfigureLog();
					log_directory.setText(AltosPreferences.logdir().getPath());
				}
			});
		c.gridwidth = 1;

		c.gridx = 0;
		c.gridy = 2;
		pane.add(configure_log, c);

		log_directory = new JTextField(AltosPreferences.logdir().getPath());
		c.gridx = 1;
		c.gridy = 2;
		c.fill = GridBagConstraints.BOTH;
		pane.add(log_directory, c);

		pack();
		setLocationRelativeTo(owner);
		setVisible(true);
	}
}

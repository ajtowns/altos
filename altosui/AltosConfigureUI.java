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
import java.beans.*;
import javax.swing.*;
import javax.swing.event.*;
import org.altusmetrum.altosuilib_2.*;

public class AltosConfigureUI
	extends AltosUIConfigure
	implements DocumentListener
{
	AltosVoice	voice;

	public JTextField	callsign_value;
	public JComboBox<String>	position_value;

	/* DocumentListener interface methods */
	public void insertUpdate(DocumentEvent e) {
		changedUpdate(e);
	}

	public void removeUpdate(DocumentEvent e) {
		changedUpdate(e);
	}

	public void changedUpdate(DocumentEvent e) {
		if (callsign_value != null)
			AltosUIPreferences.set_callsign(callsign_value.getText());
	}

	public void add_voice() {

		/* Voice settings */
		pane.add(new JLabel("Voice"), constraints(0, 1));

		JRadioButton enable_voice = new JRadioButton("Enable", AltosUIPreferences.voice());
		enable_voice.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					JRadioButton item = (JRadioButton) e.getSource();
					boolean enabled = item.isSelected();
					AltosUIPreferences.set_voice(enabled);
					if (enabled)
						voice.speak_always("Enable voice.");
					else
						voice.speak_always("Disable voice.");
				}
			});
		pane.add(enable_voice, constraints(1, 1));
		enable_voice.setToolTipText("Enable/Disable all audio in-flight announcements");

		JButton test_voice = new JButton("Test Voice");
		test_voice.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					voice.speak("That's one small step for man; one giant leap for mankind.");
				}
			});
		pane.add(test_voice, constraints(2, 1));
		test_voice.setToolTipText("Play a stock audio clip to check volume");
		row++;
	}

	public void add_callsign() {
		/* Callsign setting */
		pane.add(new JLabel("Callsign"), constraints(0, 1));

		callsign_value = new JTextField(AltosUIPreferences.callsign());
		callsign_value.getDocument().addDocumentListener(this);
		callsign_value.setToolTipText("Callsign sent in packet mode");
		pane.add(callsign_value, constraints(1, 2, GridBagConstraints.BOTH));
		row++;
	}

	public void add_bluetooth() {
		JButton manage_bluetooth = new JButton("Manage Bluetooth");
		manage_bluetooth.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					AltosBTManage.show(owner, AltosBTKnown.bt_known());
				}
			});
		pane.add(manage_bluetooth, constraints(0, 2));
		/* in the same row as add_frequencies, so don't bump row */
	}

	public void add_frequencies() {
		JButton manage_frequencies = new JButton("Manage Frequencies");
		manage_frequencies.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					AltosConfigFreqUI.show(owner);
				}
			});
		manage_frequencies.setToolTipText("Configure which values are shown in frequency menus");
		pane.add(manage_frequencies, constraints(2, 1));
		row++;
	}

	final static String[] position_names = {
		"Top left",
		"Top",
		"Top right",
		"Left",
		"Center",
		"Right",
		"Bottom left",
		"Bottom",
		"Bottom right",
	};

	public void add_position() {
		pane.add(new JLabel ("Menu position"), constraints(0, 1));

		position_value = new JComboBox<String>(position_names);
		position_value.setMaximumRowCount(position_names.length);
		int position = AltosUIPreferences.position();
		position_value.setSelectedIndex(position);
		position_value.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					int	position = position_value.getSelectedIndex();
					AltosUIPreferences.set_position(position);
				}
			});
		pane.add(position_value, constraints(1, 2, GridBagConstraints.BOTH));
		position_value.setToolTipText("Position of main AltosUI window");
		row++;
	}

	public AltosConfigureUI(JFrame owner, AltosVoice voice) {
		super(owner);

		this.voice = voice;
	}
}

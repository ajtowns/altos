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
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.table.*;
import javax.swing.event.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.util.concurrent.LinkedBlockingQueue;
import javax.swing.plaf.basic.*;
import org.altusmetrum.AltosLib.*;

class DelegatingRenderer implements ListCellRenderer {

	// ...
	public static void install(JComboBox comboBox) {
		DelegatingRenderer renderer = new DelegatingRenderer(comboBox);
		renderer.initialise();
		comboBox.setRenderer(renderer);
	}

	// ...
	private final JComboBox comboBox;

	// ...
	private ListCellRenderer delegate;

	// ...
	private DelegatingRenderer(JComboBox comboBox) {
		this.comboBox = comboBox;
	}

	// ...
	private void initialise() {
		delegate = new JComboBox().getRenderer();
		comboBox.addPropertyChangeListener("UI", new PropertyChangeListener() {

				public void propertyChange(PropertyChangeEvent evt) {
					delegate = new JComboBox().getRenderer();
				}
			});
	}

	// ...
	public Component getListCellRendererComponent(JList list,
						      Object value, int index, boolean isSelected, boolean cellHasFocus) {

		return delegate.getListCellRendererComponent(list,
							     ((UIManager.LookAndFeelInfo) value).getName(),
							     index, isSelected, cellHasFocus);
	}
}

public class AltosConfigureUI
	extends AltosDialog
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

	JRadioButton	imperial_units;

	JLabel		font_size_label;
	JComboBox	font_size_value;

	JLabel		look_and_feel_label;
	JComboBox	look_and_feel_value;

	JRadioButton	serial_debug;

	JButton		manage_bluetooth;
	JButton		manage_frequencies;

	final static String[] font_size_names = { "Small", "Medium", "Large" };

	/* DocumentListener interface methods */
	public void changedUpdate(DocumentEvent e) {
		AltosUIPreferences.set_callsign(callsign_value.getText());
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

		enable_voice = new JRadioButton("Enable", AltosUIPreferences.voice());
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

		configure_log = new JButton(AltosUIPreferences.logdir().getPath());
		configure_log.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					AltosUIPreferences.ConfigureLog();
					configure_log.setText(AltosUIPreferences.logdir().getPath());
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

		callsign_value = new JTextField(AltosUIPreferences.callsign());
		callsign_value.getDocument().addDocumentListener(this);
		c.gridx = 1;
		c.gridy = row++;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.BOTH;
		c.anchor = GridBagConstraints.WEST;
		pane.add(callsign_value, c);
		callsign_value.setToolTipText("Callsign sent in packet mode");

		/* Imperial units setting */
		c.gridx = 0;
		c.gridy = row;
		c.gridwidth = 1;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		pane.add(new JLabel("Imperial Units"), c);

		imperial_units = new JRadioButton("Enable", AltosUIPreferences.imperial_units());
		imperial_units.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					JRadioButton item = (JRadioButton) e.getSource();
					boolean enabled = item.isSelected();
					AltosUIPreferences.set_imperial_units(enabled);
				}
			});
		imperial_units.setToolTipText("Use Imperial units instead of metric");

		c.gridx = 1;
		c.gridy = row++;
		c.gridwidth = 3;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		pane.add(imperial_units, c);

		/* Font size setting */
		c.gridx = 0;
		c.gridy = row;
		c.gridwidth = 1;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		pane.add(new JLabel("Font size"), c);

		font_size_value = new JComboBox(font_size_names);
		int font_size = AltosUIPreferences.font_size();
		font_size_value.setSelectedIndex(font_size - Altos.font_size_small);
		font_size_value.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					int	size = font_size_value.getSelectedIndex() + Altos.font_size_small;

					AltosUIPreferences.set_font_size(size);
				}
			});
		c.gridx = 1;
		c.gridy = row++;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.BOTH;
		c.anchor = GridBagConstraints.WEST;
		pane.add(font_size_value, c);
		font_size_value.setToolTipText("Font size used in telemetry window");

		/* Look & Feel setting */
		c.gridx = 0;
		c.gridy = row;
		c.gridwidth = 1;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		pane.add(new JLabel("Look & feel"), c);

		class LookAndFeelRenderer extends BasicComboBoxRenderer implements ListCellRenderer {

			public LookAndFeelRenderer() {
				super();
			}

			public Component getListCellRendererComponent(
				JList list,
				Object value,
				int index,
				boolean isSelected,
				boolean cellHasFocus)
			{
				super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);
				setText(((UIManager.LookAndFeelInfo) value).getName());
				return this;
			}
		}

		final UIManager.LookAndFeelInfo[] look_and_feels = UIManager.getInstalledLookAndFeels();

		look_and_feel_value = new JComboBox(look_and_feels);

		DelegatingRenderer.install(look_and_feel_value);

		String look_and_feel  = AltosUIPreferences.look_and_feel();
		for (int i = 0; i < look_and_feels.length; i++)
			if (look_and_feel.equals(look_and_feels[i].getClassName()))
				look_and_feel_value.setSelectedIndex(i);

		look_and_feel_value.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					int	id = look_and_feel_value.getSelectedIndex();

					AltosUIPreferences.set_look_and_feel(look_and_feels[id].getClassName());
				}
			});
		c.gridx = 1;
		c.gridy = row++;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.BOTH;
		c.anchor = GridBagConstraints.WEST;
		pane.add(look_and_feel_value, c);
		look_and_feel_value.setToolTipText("Look&feel used for new windows");

		/* Serial debug setting */
		c.gridx = 0;
		c.gridy = row;
		c.gridwidth = 1;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		pane.add(new JLabel("Serial Debug"), c);

		serial_debug = new JRadioButton("Enable", AltosUIPreferences.serial_debug());
		serial_debug.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					JRadioButton item = (JRadioButton) e.getSource();
					boolean enabled = item.isSelected();
					AltosUIPreferences.set_serial_debug(enabled);
				}
			});
		serial_debug.setToolTipText("Enable/Disable USB I/O getting sent to the console");

		c.gridx = 1;
		c.gridy = row++;
		c.gridwidth = 3;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		pane.add(serial_debug, c);

		manage_bluetooth = new JButton("Manage Bluetooth");
		manage_bluetooth.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					AltosBTManage.show(owner, AltosBTKnown.bt_known());
				}
			});
		c.gridx = 0;
		c.gridy = row;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		pane.add(manage_bluetooth, c);

		manage_frequencies = new JButton("Manage Frequencies");
		manage_frequencies.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					AltosConfigFreqUI.show(owner);
				}
			});
		manage_frequencies.setToolTipText("Configure which values are shown in frequency menus");
		c.gridx = 2;
		c.gridx = 2;
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

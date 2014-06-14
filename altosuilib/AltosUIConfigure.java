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
import java.beans.*;
import javax.swing.*;
import javax.swing.event.*;

class DelegatingRenderer implements ListCellRenderer<Object> {

	// ...
	public static void install(JComboBox<Object> comboBox) {
		DelegatingRenderer renderer = new DelegatingRenderer(comboBox);
		renderer.initialise();
		comboBox.setRenderer(renderer);
	}

	// ...
	private final JComboBox comboBox;

	// ...
	private ListCellRenderer<? super Object> delegate;

	// ...
	private DelegatingRenderer(JComboBox comboBox) {
		this.comboBox = comboBox;
	}

	// ...
	private void initialise() {
		JComboBox<Object> c = new JComboBox<Object>();
		delegate = c.getRenderer();
		comboBox.addPropertyChangeListener("UI", new PropertyChangeListener() {

				public void propertyChange(PropertyChangeEvent evt) {
					delegate = new JComboBox<Object>().getRenderer();
				}
			});
	}

	// ...
	public Component getListCellRendererComponent(JList<?> list,
						      Object value, int index, boolean isSelected, boolean cellHasFocus) {

		return delegate.getListCellRendererComponent(list,
							     ((UIManager.LookAndFeelInfo)value).getName(),
							     index, isSelected, cellHasFocus);
	}
}

public class AltosUIConfigure
	extends AltosUIDialog
{
	public JFrame		owner;
	public Container	pane;

	public int		row;

	final static String[] font_size_names = { "Small", "Medium", "Large" };

	public GridBagConstraints constraints (int x, int width, int fill) {
		GridBagConstraints c = new GridBagConstraints();
		Insets insets = new Insets(4, 4, 4, 4);

		c.insets = insets;
		c.fill = fill;
		if (width == 3)
			c.anchor = GridBagConstraints.CENTER;
		else if (x == 2)
			c.anchor = GridBagConstraints.EAST;
		else
			c.anchor = GridBagConstraints.WEST;
		c.gridx = x;
		c.gridwidth = width;
		c.gridy = row;
		return c;
	}

	public GridBagConstraints constraints(int x, int width) {
		return constraints(x, width, GridBagConstraints.NONE);
	}

	public void add_voice() {
	}

	public void add_log_dir() {
		/* Log directory settings */
		pane.add(new JLabel("Log Directory"), constraints(0, 1));

		final JButton configure_log = new JButton(AltosUIPreferences.logdir().getPath());
		configure_log.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					AltosUIPreferences.ConfigureLog();
					configure_log.setText(AltosUIPreferences.logdir().getPath());
				}
			});
		pane.add(configure_log, constraints(1, 2));
		configure_log.setToolTipText("Which directory flight logs are stored in");
		row++;
	}

	public void add_callsign() {
	}

	public void add_units() {
		/* Imperial units setting */
		pane.add(new JLabel("Imperial Units"), constraints(0, 1));

		JRadioButton imperial_units = new JRadioButton("Enable", AltosUIPreferences.imperial_units());
		imperial_units.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					JRadioButton item = (JRadioButton) e.getSource();
					boolean enabled = item.isSelected();
					AltosUIPreferences.set_imperial_units(enabled);
				}
			});
		imperial_units.setToolTipText("Use Imperial units instead of metric");
		pane.add(imperial_units, constraints(1, 2));
		row++;
	}

	public void add_font_size() {
		/* Font size setting */
		pane.add(new JLabel("Font size"), constraints(0, 1));

		final JComboBox<String> font_size_value = new JComboBox<String>(font_size_names);
		int font_size = AltosUIPreferences.font_size();
		font_size_value.setSelectedIndex(font_size - AltosUILib.font_size_small);
		font_size_value.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					int	size = font_size_value.getSelectedIndex() + AltosUILib.font_size_small;

					AltosUIPreferences.set_font_size(size);
				}
			});
		pane.add(font_size_value, constraints(1, 2, GridBagConstraints.BOTH));
		font_size_value.setToolTipText("Font size used in telemetry window");
		row++;
	}

	public void add_look_and_feel() {
		/* Look & Feel setting */
		pane.add(new JLabel("Look & feel"), constraints(0, 1));

		/*
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
		*/

		final UIManager.LookAndFeelInfo[] look_and_feels = UIManager.getInstalledLookAndFeels();

		final JComboBox<Object> look_and_feel_value = new JComboBox<Object>(look_and_feels);

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
		pane.add(look_and_feel_value, constraints(1, 2, GridBagConstraints.BOTH));
		look_and_feel_value.setToolTipText("Look&feel used for new windows");
		row++;
	}

	public void add_position () {
	}

	public void add_serial_debug() {
		/* Serial debug setting */
		pane.add(new JLabel("Serial Debug"), constraints(0, 1));

		JRadioButton serial_debug = new JRadioButton("Enable", AltosUIPreferences.serial_debug());
		serial_debug.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					JRadioButton item = (JRadioButton) e.getSource();
					boolean enabled = item.isSelected();
					AltosUIPreferences.set_serial_debug(enabled);
				}
			});
		serial_debug.setToolTipText("Enable/Disable USB I/O getting sent to the console");
		pane.add(serial_debug, constraints(1,2));
		row++;
	}

	public void add_bluetooth() {
	}

	public void add_frequencies() {
	}

	public AltosUIConfigure(JFrame in_owner, String name, String label) {
		super(in_owner, name, false);

		owner = in_owner;
		pane = getContentPane();
		pane.setLayout(new GridBagLayout());

		row = 0;

		/* Nice label at the top */
		pane.add(new JLabel (label),
			 constraints(0, 3));
		row++;

		pane.add(new JLabel (String.format("AltOS version %s", AltosUIVersion.version)),
			 constraints(0, 3));
		row++;

		add_voice();
		add_log_dir();
		add_callsign();
		add_units();
		add_serial_debug();
		add_font_size();
		add_look_and_feel();
		add_position();
		add_bluetooth();
		add_frequencies();

		/* And a close button at the bottom */
		JButton close = new JButton("Close");
		close.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					setVisible(false);
				}
			});
		pane.add(close, constraints(0, 3));

		pack();
		setLocationRelativeTo(owner);
		setVisible(true);
	}

	public AltosUIConfigure(JFrame in_owner) {
		this(in_owner, "Configure AltosUI", "Configure AltOS UI");
	}
}

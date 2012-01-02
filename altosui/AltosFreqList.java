/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

public class AltosFreqList extends JComboBox {

	String	product;
	int	serial;
	int	calibrate;

	public void set_frequency(double new_frequency) {
		int i;
		for (i = 0; i < getItemCount(); i++) {
			AltosFrequency	f = (AltosFrequency) getItemAt(i);
			
			if (f.close(new_frequency)) {
				setSelectedIndex(i);
				return;
			}
		}
		for (i = 0; i < getItemCount(); i++) {
			AltosFrequency	f = (AltosFrequency) getItemAt(i);
			
			if (new_frequency < f.frequency)
				break;
		}
		String	description = String.format("%s serial %d", product, serial);
		AltosFrequency	frequency = new AltosFrequency(new_frequency, description);
		AltosUIPreferences.add_common_frequency(frequency);
		insertItemAt(frequency, i);
		setMaximumRowCount(getItemCount());
	}

	public void set_product(String new_product) {
		product = new_product;
	}
		
	public void set_serial(int new_serial) {
		serial = new_serial;
	}

	public double frequency() {
		AltosFrequency	f = (AltosFrequency) getSelectedItem();
		if (f != null)
			return f.frequency;
		return 434.550;
	}

	public AltosFreqList () {
		super(AltosUIPreferences.common_frequencies());
		setMaximumRowCount(getItemCount());
		setEditable(false);
		product = "Unknown";
		serial = 0;
	}

	public AltosFreqList(double in_frequency) {
		this();
		set_frequency(in_frequency);
	}
}

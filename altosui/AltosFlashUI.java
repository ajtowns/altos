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

public class AltosFlashUI
	extends JDialog
	implements Runnable, ActionListener
{
	Container	pane;
	Box		box;
	JLabel		serial_label;
	JLabel		serial_value;
	JLabel		file_label;
	JLabel		file_value;
	JProgressBar	pbar;
	JButton		cancel;

	File		file;
	Thread		thread;
	JFrame		frame;
	AltosDevice	debug_dongle;
	AltosFlash	flash;

	public void actionPerformed(ActionEvent e) {
		if (e.getSource() == cancel) {
			abort();
			dispose();
		} else {
			String	cmd = e.getActionCommand();
			if (cmd.equals("done"))
				;
			else if (cmd.equals("start")) {
				setVisible(true);
			} else {
				pbar.setValue(e.getID());
				pbar.setString(cmd);
			}
		}
	}

	public void run() {
		try {
			flash = new AltosFlash(file, debug_dongle);
			flash.addActionListener(this);
			AltosRomconfigUI romconfig_ui = new AltosRomconfigUI (frame);

			romconfig_ui.set(flash.romconfig());
			AltosRomconfig romconfig = romconfig_ui.showDialog();

			if (romconfig != null && romconfig.valid()) {
				flash.set_romconfig(romconfig);
				serial_value.setText(String.format("%d",
								   flash.romconfig().serial_number));
				file_value.setText(file.toString());
				setVisible(true);
				flash.flash();
				flash = null;
			}
		} catch (FileNotFoundException ee) {
			JOptionPane.showMessageDialog(frame,
						      "Cannot open image",
						      file.toString(),
						      JOptionPane.ERROR_MESSAGE);
		} catch (AltosSerialInUseException si) {
			JOptionPane.showMessageDialog(frame,
						      String.format("Device \"%s\" already in use",
								    debug_dongle.toShortString()),
						      "Device in use",
						      JOptionPane.ERROR_MESSAGE);
		} catch (IOException e) {
			JOptionPane.showMessageDialog(frame,
						      e.getMessage(),
						      file.toString(),
						      JOptionPane.ERROR_MESSAGE);
		} catch (InterruptedException ie) {
		} finally {
			abort();
		}
		dispose();
	}

	public void abort() {
		if (flash != null)
			flash.abort();
	}

	public void build_dialog() {
		GridBagConstraints c;
		Insets il = new Insets(4,4,4,4);
		Insets ir = new Insets(4,4,4,4);

		pane = getContentPane();
		pane.setLayout(new GridBagLayout());

		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 0;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		serial_label = new JLabel("Serial:");
		pane.add(serial_label, c);

		c = new GridBagConstraints();
		c.gridx = 1; c.gridy = 0;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		serial_value = new JLabel("");
		pane.add(serial_value, c);

		c = new GridBagConstraints();
		c.fill = GridBagConstraints.NONE;
		c.gridx = 0; c.gridy = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		file_label = new JLabel("File:");
		pane.add(file_label, c);

		c = new GridBagConstraints();
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.gridx = 1; c.gridy = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		file_value = new JLabel("");
		pane.add(file_value, c);

		pbar = new JProgressBar();
		pbar.setMinimum(0);
		pbar.setMaximum(100);
		pbar.setValue(0);
		pbar.setString("");
		pbar.setStringPainted(true);
		pbar.setPreferredSize(new Dimension(600, 20));
		c = new GridBagConstraints();
		c.fill = GridBagConstraints.HORIZONTAL;
		c.anchor = GridBagConstraints.CENTER;
		c.gridx = 0; c.gridy = 2;
		c.gridwidth = GridBagConstraints.REMAINDER;
		Insets ib = new Insets(4,4,4,4);
		c.insets = ib;
		pane.add(pbar, c);

		cancel = new JButton("Cancel");
		c = new GridBagConstraints();
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.gridx = 0; c.gridy = 3;
		c.gridwidth = GridBagConstraints.REMAINDER;
		Insets ic = new Insets(4,4,4,4);
		c.insets = ic;
		pane.add(cancel, c);
		cancel.addActionListener(this);
		pack();
		setLocationRelativeTo(frame);
	}

	public AltosFlashUI(JFrame in_frame) {
		super(in_frame, "Program Altusmetrum Device", false);

		frame = in_frame;

		build_dialog();

		debug_dongle = AltosDeviceDialog.show(frame, AltosDevice.product_any);

		if (debug_dongle == null)
			return;

		JFileChooser	hexfile_chooser = new JFileChooser();

		File firmwaredir = AltosPreferences.firmwaredir();
		if (firmwaredir != null)
			hexfile_chooser.setCurrentDirectory(firmwaredir);

		hexfile_chooser.setDialogTitle("Select Flash Image");
		hexfile_chooser.setFileFilter(new FileNameExtensionFilter("Flash Image", "ihx"));
		int returnVal = hexfile_chooser.showOpenDialog(frame);

		if (returnVal != JFileChooser.APPROVE_OPTION)
			return;

		file = hexfile_chooser.getSelectedFile();

		if (file != null)
			AltosPreferences.set_firmwaredir(file.getParentFile());

		thread = new Thread(this);
		thread.start();
	}
}
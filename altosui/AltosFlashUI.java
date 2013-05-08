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
import java.io.*;
import java.util.concurrent.*;
import org.altusmetrum.altoslib_1.*;
import org.altusmetrum.altosuilib_1.*;

public class AltosFlashUI
	extends AltosUIDialog
	implements ActionListener
{
	Container	pane;
	Box		box;
	JLabel		serial_label;
	JLabel		serial_value;
	JLabel		file_label;
	JLabel		file_value;
	JProgressBar	pbar;
	JButton		cancel;

	JFrame		frame;

	// Hex file with rom image
	File		file;

	// Debug connection
	AltosDevice	debug_dongle;

	// Desired Rom configuration
	AltosRomconfig	rom_config;

	// Flash controller
	AltosFlash	flash;

	public void actionPerformed(ActionEvent e) {
		if (e.getSource() == cancel) {
			if (flash != null)
				flash.abort();
			setVisible(false);
			dispose();
		} else {
			String	cmd = e.getActionCommand();
			if (e.getID() == -1) {
				JOptionPane.showMessageDialog(frame,
							      e.getActionCommand(),
							      file.toString(),
							      JOptionPane.ERROR_MESSAGE);
				setVisible(false);
				dispose();
			} else if (cmd.equals("done")) {
				setVisible(false);
				dispose();
			} else if (cmd.equals("start")) {
				setVisible(true);
			} else {
				pbar.setValue(e.getID());
				pbar.setString(cmd);
			}
		}
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
		file_value = new JLabel(file.toString());
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

	void set_serial(int serial_number) {
		serial_value.setText(String.format("%d", serial_number));
	}

	boolean select_source_file() {
		JFileChooser	hexfile_chooser = new JFileChooser();

		File firmwaredir = AltosUIPreferences.firmwaredir();
		if (firmwaredir != null)
			hexfile_chooser.setCurrentDirectory(firmwaredir);

		hexfile_chooser.setDialogTitle("Select Flash Image");
		hexfile_chooser.setFileFilter(new FileNameExtensionFilter("Flash Image", "ihx"));
		int returnVal = hexfile_chooser.showOpenDialog(frame);

		if (returnVal != JFileChooser.APPROVE_OPTION)
			return false;
		file = hexfile_chooser.getSelectedFile();
		if (file == null)
			return false;
		AltosUIPreferences.set_firmwaredir(file.getParentFile());
		return true;
	}

	boolean select_debug_dongle() {
		debug_dongle = AltosDeviceUIDialog.show(frame, Altos.product_any);

		if (debug_dongle == null)
			return false;
		return true;
	}

	boolean update_rom_config_info(AltosRomconfig existing_config) {
		AltosRomconfig	new_config;
		new_config = AltosRomconfigUI.show(frame, existing_config);
		if (new_config == null)
			return false;
		rom_config = new_config;
		set_serial(rom_config.serial_number);
		setVisible(true);
		return true;
	}

	void exception (Exception e) {
		if (e instanceof FileNotFoundException) {
			JOptionPane.showMessageDialog(frame,
						      ((FileNotFoundException) e).getMessage(),
						      "Cannot open file",
						      JOptionPane.ERROR_MESSAGE);
		} else if (e instanceof AltosSerialInUseException) {
			JOptionPane.showMessageDialog(frame,
						      String.format("Device \"%s\" already in use",
								    debug_dongle.toShortString()),
						      "Device in use",
						      JOptionPane.ERROR_MESSAGE);
		} else if (e instanceof IOException) {
			JOptionPane.showMessageDialog(frame,
						      e.getMessage(),
						      file.toString(),
						      JOptionPane.ERROR_MESSAGE);
		}
	}

	class flash_task implements Runnable, AltosFlashListener {
		AltosFlashUI	ui;
		Thread		t;
		AltosFlash	flash;

		public void position(String in_s, int in_percent) {
			final String s = in_s;
			final int percent = in_percent;
			Runnable r = new Runnable() {
					public void run() {
						try {
							ui.actionPerformed(new ActionEvent(this,
											   percent,
											   s));
						} catch (Exception ex) {
						}
					}
				};
			SwingUtilities.invokeLater(r);
		}

		public void run () {
			try {
				flash = new AltosFlash(ui.file, new AltosSerial(ui.debug_dongle), this);

				final AltosRomconfig	current_config = flash.romconfig();

				final Semaphore await_rom_config = new Semaphore(0);
				SwingUtilities.invokeLater(new Runnable() {
						public void run() {
							ui.flash = flash;
							ui.update_rom_config_info(current_config);
							await_rom_config.release();
						}
					});
				await_rom_config.acquire();

				if (ui.rom_config != null) {
					flash.set_romconfig(ui.rom_config);
					flash.flash();
				}
			} catch (InterruptedException ee) {
				final Exception	e = ee;
				SwingUtilities.invokeLater(new Runnable() {
						public void run() {
							ui.exception(e);
						}
					});
			} catch (IOException ee) {
				final Exception	e = ee;
				SwingUtilities.invokeLater(new Runnable() {
						public void run() {
							ui.exception(e);
						}
					});
			} catch (AltosSerialInUseException ee) {
				final Exception	e = ee;
				SwingUtilities.invokeLater(new Runnable() {
						public void run() {
							ui.exception(e);
						}
					});
			} finally {
				if (flash != null)
					flash.close();
			}
		}

		public flash_task(AltosFlashUI in_ui) {
			ui = in_ui;
			t = new Thread(this);
			t.start();
		}
	}

	flash_task	flasher;

	/*
	 * Execute the steps for flashing
	 * a device. Note that this returns immediately;
	 * this dialog is not modal
	 */
	void showDialog() {
		if (!select_debug_dongle())
			return;
		if (!select_source_file())
			return;
		build_dialog();
		flash_task	f = new flash_task(this);
	}

	static void show(JFrame frame) {
		AltosFlashUI	ui = new AltosFlashUI(frame);

		ui.showDialog();
	}

	public AltosFlashUI(JFrame in_frame) {
		super(in_frame, "Program Altusmetrum Device", false);

		frame = in_frame;
	}
}
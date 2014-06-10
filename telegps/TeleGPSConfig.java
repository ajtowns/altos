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

package org.altusmetrum.telegps;

import java.awt.event.*;
import javax.swing.*;
import java.io.*;
import java.util.concurrent.*;
import java.text.*;
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

public class TeleGPSConfig implements ActionListener {

	class int_ref {
		int	value;

		public int get() {
			return value;
		}
		public void set(int i) {
			value = i;
		}
		public int_ref(int i) {
			value = i;
		}
	}

	class string_ref {
		String	value;

		public String get() {
			return value;
		}
		public void set(String i) {
			value = i;
		}
		public string_ref(String i) {
			value = i;
		}
	}

	JFrame		owner;
	AltosDevice	device;
	AltosSerial	serial_line;

	AltosConfigData	data;
	TeleGPSConfigUI	config_ui;
	boolean		serial_started;
	boolean		made_visible;

	void start_serial() throws InterruptedException, TimeoutException {
		serial_started = true;
	}

	void stop_serial() throws InterruptedException {
		if (!serial_started)
			return;
		serial_started = false;
	}

	void update_ui() {
		data.set_values(config_ui);
		config_ui.set_clean();
		if (!made_visible) {
			made_visible = true;
			config_ui.make_visible();
		}
	}

	int	pyro;

	final static int	serial_mode_read = 0;
	final static int	serial_mode_save = 1;
	final static int	serial_mode_reboot = 2;

	class SerialData implements Runnable {
		TeleGPSConfig	config;
		int		serial_mode;

		void callback(String in_cmd) {
			final String cmd = in_cmd;
			Runnable r = new Runnable() {
					public void run() {
						if (cmd.equals("abort")) {
							abort();
						} else if (cmd.equals("all finished")) {
							if (serial_line != null)
								update_ui();
						}
					}
				};
			SwingUtilities.invokeLater(r);
		}

		void get_data() {
			data = null;
			try {
				start_serial();
				data = new AltosConfigData(config.serial_line);
			} catch (InterruptedException ie) {
			} catch (TimeoutException te) {
				try {
					stop_serial();
					callback("abort");
				} catch (InterruptedException ie) {
				}
			} finally {
				try {
					stop_serial();
				} catch (InterruptedException ie) {
				}
			}
			callback("all finished");
		}

		void save_data() {
			try {
				start_serial();
				data.save(serial_line, false);
			} catch (InterruptedException ie) {
			} catch (TimeoutException te) {
			} finally {
				try {
					stop_serial();
				} catch (InterruptedException ie) {
				}
			}
		}

		void reboot() {
			try {
				start_serial();
				serial_line.printf("r eboot\n");
				serial_line.flush_output();
			} catch (InterruptedException ie) {
			} catch (TimeoutException te) {
			} finally {
				try {
					stop_serial();
					serial_line.close();
				} catch (InterruptedException ie) {
				}
			}
		}

		public void run () {
			switch (serial_mode) {
			case serial_mode_save:
				save_data();
				/* fall through ... */
			case serial_mode_read:
				get_data();
				break;
			case serial_mode_reboot:
				reboot();
				break;
			}
		}

		public SerialData(TeleGPSConfig in_config, int in_serial_mode) {
			config = in_config;
			serial_mode = in_serial_mode;
		}
	}

	void run_serial_thread(int serial_mode) {
		SerialData	sd = new SerialData(this, serial_mode);
		Thread		st = new Thread(sd);
		st.start();
	}

	void init_ui () throws InterruptedException, TimeoutException {
		config_ui = new TeleGPSConfigUI(owner);
		config_ui.addActionListener(this);
		serial_line.set_frame(owner);
		set_ui();
	}

	void abort() {
		if (serial_line != null) {
			serial_line.close();
			serial_line = null;
		}
		JOptionPane.showMessageDialog(owner,
					      String.format("Connection to \"%s\" failed",
							    device.toShortString()),
					      "Connection Failed",
					      JOptionPane.ERROR_MESSAGE);
		config_ui.setVisible(false);
	}

	void set_ui() throws InterruptedException, TimeoutException {
		if (serial_line != null)
			run_serial_thread(serial_mode_read);
		else
			update_ui();
	}

	double frequency() {
		return AltosConvert.radio_to_frequency(data.radio_frequency,
						       data.radio_setting,
						       data.radio_calibration,
						       data.radio_channel);
	}

	void save_data() {

		try {
			/* bounds check stuff */
			if (config_ui.flight_log_max() > data.log_space()/1024) {
				JOptionPane.showMessageDialog(owner,
							      String.format("Requested flight log, %dk, is larger than the available space, %dk.\n",
									    config_ui.flight_log_max(),
									    data.log_space()/1024),
							      "Maximum Flight Log Too Large",
							      JOptionPane.ERROR_MESSAGE);
				return;
			}

			/* Pull data out of the UI and stuff back into our local data record */

			data.get_values(config_ui);
			run_serial_thread(serial_mode_save);
		} catch (AltosConfigDataException ae) {
			JOptionPane.showMessageDialog(owner,
						      ae.getMessage(),
						      "Configuration Data Error",
						      JOptionPane.ERROR_MESSAGE);
		}
	}

	public void actionPerformed(ActionEvent e) {
		String	cmd = e.getActionCommand();
		try {
			if (cmd.equals("Save")) {
				save_data();
			} else if (cmd.equals("Reset")) {
				set_ui();
			} else if (cmd.equals("Reboot")) {
				if (serial_line != null)
					run_serial_thread(serial_mode_reboot);
			} else if (cmd.equals("Close")) {
				if (serial_line != null)
					serial_line.close();
			}
		} catch (InterruptedException ie) {
			abort();
		} catch (TimeoutException te) {
			abort();
		}
	}

	public TeleGPSConfig(JFrame given_owner) {
		owner = given_owner;

		device = AltosDeviceUIDialog.show(owner, AltosLib.product_telegps);
		if (device != null) {
			try {
				serial_line = new AltosSerial(device);
				try {
					init_ui();
				} catch (InterruptedException ie) {
					abort();
				} catch (TimeoutException te) {
					abort();
				}
			} catch (FileNotFoundException ee) {
				JOptionPane.showMessageDialog(owner,
							      ee.getMessage(),
							      "Cannot open target device",
							      JOptionPane.ERROR_MESSAGE);
			} catch (AltosSerialInUseException si) {
				JOptionPane.showMessageDialog(owner,
							      String.format("Device \"%s\" already in use",
									    device.toShortString()),
							      "Device in use",
							      JOptionPane.ERROR_MESSAGE);
			}
		}
	}
}

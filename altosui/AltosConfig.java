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

import java.awt.event.*;
import javax.swing.*;
import java.io.*;
import java.util.concurrent.*;
import org.altusmetrum.AltosLib.*;
import java.text.*;

public class AltosConfig implements ActionListener {

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
	boolean		remote;

	AltosConfigData	data;
	AltosConfigUI	config_ui;
	boolean		serial_started;
	boolean		made_visible;

	void start_serial() throws InterruptedException, TimeoutException {
		serial_started = true;
		if (remote)
			serial_line.start_remote();
	}

	void stop_serial() throws InterruptedException {
		if (!serial_started)
			return;
		serial_started = false;
		if (remote)
			serial_line.stop_remote();
	}

	int log_limit() {
		if (data.storage_size > 0 && data.storage_erase_unit > 0) {
			int	log_limit = data.storage_size - data.storage_erase_unit;
			if (log_limit > 0)
				return log_limit / 1024;
		}
		return 1024;
	}

	void update_ui() {
		config_ui.set_serial(data.serial);
		config_ui.set_product(data.product);
		config_ui.set_version(data.version);
		config_ui.set_main_deploy(data.main_deploy);
		config_ui.set_apogee_delay(data.apogee_delay);
		config_ui.set_apogee_lockout(data.apogee_lockout);
		config_ui.set_radio_calibration(data.radio_calibration);
		config_ui.set_radio_frequency(frequency());
		boolean max_enabled = true;
		switch (data.log_format) {
		case Altos.AO_LOG_FORMAT_TINY:
			max_enabled = false;
			break;
		default:
			if (data.stored_flight >= 0)
				max_enabled = false;
			break;
		}
		config_ui.set_flight_log_max_enabled(max_enabled);
		config_ui.set_radio_enable(data.radio_enable);
		config_ui.set_flight_log_max_limit(log_limit());
		config_ui.set_flight_log_max(data.flight_log_max);
		config_ui.set_ignite_mode(data.ignite_mode);
		config_ui.set_pad_orientation(data.pad_orientation);
		config_ui.set_callsign(data.callsign);
		config_ui.set_pyros(data.pyros);
		config_ui.set_has_pyro(data.npyro > 0);
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
		AltosConfig	config;
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
				double frequency = frequency();
				boolean has_frequency = data.radio_frequency > 0;
				boolean has_setting = data.radio_setting > 0;
				start_serial();
				serial_line.printf("c m %d\n", data.main_deploy);
				serial_line.printf("c d %d\n", data.apogee_delay);
				serial_line.printf("c L %d\n", data.apogee_lockout);
				if (!remote)
					serial_line.printf("c f %d\n", data.radio_calibration);
				serial_line.set_radio_frequency(frequency,
								has_frequency,
								has_setting,
								data.radio_calibration);
				if (remote) {
					serial_line.stop_remote();
					serial_line.set_radio_frequency(frequency);
					AltosUIPreferences.set_frequency(device.getSerial(), frequency);
					serial_line.start_remote();
				}
				serial_line.printf("c c %s\n", data.callsign);
				if (data.flight_log_max != 0)
					serial_line.printf("c l %d\n", data.flight_log_max);
				if (data.radio_enable >= 0)
					serial_line.printf("c e %d\n", data.radio_enable);
				if (data.ignite_mode >= 0)
					serial_line.printf("c i %d\n", data.ignite_mode);
				if (data.pad_orientation >= 0)
					serial_line.printf("c o %d\n", data.pad_orientation);
				if (data.pyros.length > 0) {
					for (int p = 0; p < data.pyros.length; p++) {
						serial_line.printf("c P %s\n",
								   data.pyros[p].toString());
					}
				}
				serial_line.printf("c w\n");
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
				} catch (InterruptedException ie) {
				}
				serial_line.close();
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

		public SerialData(AltosConfig in_config, int in_serial_mode) {
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
		config_ui = new AltosConfigUI(owner, remote);
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

	void set_frequency(double freq) {
		int	frequency = data.radio_frequency;
		int	setting = data.radio_setting;

		if (frequency > 0) {
			data.radio_frequency = (int) Math.floor (freq * 1000 + 0.5);
			data.radio_channel = 0;
		} else if (setting > 0) {
			data.radio_setting =AltosConvert.radio_frequency_to_setting(freq,
										    data.radio_calibration);
			data.radio_channel = 0;
		} else {
			data.radio_channel = AltosConvert.radio_frequency_to_channel(freq);
		}
	}

	void save_data() {

		/* bounds check stuff */
		if (config_ui.flight_log_max() > log_limit()) {
			JOptionPane.showMessageDialog(owner,
						      String.format("Requested flight log, %dk, is larger than the available space, %dk.\n",
								    config_ui.flight_log_max(),
								    log_limit()),
						      "Maximum Flight Log Too Large",
						      JOptionPane.ERROR_MESSAGE);
			return;
		}

		data.main_deploy = config_ui.main_deploy();
		data.apogee_delay = config_ui.apogee_delay();
		data.apogee_lockout = config_ui.apogee_lockout();
		data.radio_calibration = config_ui.radio_calibration();
		set_frequency(config_ui.radio_frequency());
		data.flight_log_max = config_ui.flight_log_max();
		if (data.radio_enable >= 0)
			data.radio_enable = config_ui.radio_enable();
		if (data.ignite_mode >= 0)
			data.ignite_mode = config_ui.ignite_mode();
		if (data.pad_orientation >= 0)
			data.pad_orientation = config_ui.pad_orientation();
		data.callsign = config_ui.callsign();
		if (data.npyro > 0) {
			data.pyros = config_ui.pyros();
		}
		run_serial_thread(serial_mode_save);
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

	public AltosConfig(JFrame given_owner) {
		owner = given_owner;

		device = AltosDeviceDialog.show(owner, Altos.product_any);
		if (device != null) {
			try {
				serial_line = new AltosSerial(device);
				try {
					if (device.matchProduct(Altos.product_basestation))
						remote = true;
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
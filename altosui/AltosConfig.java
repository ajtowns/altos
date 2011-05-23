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
import java.util.concurrent.*;

import libaltosJNI.*;

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
	int_ref		serial;
	int_ref		main_deploy;
	int_ref		apogee_delay;
	int_ref		radio_channel;
	int_ref		radio_calibration;
	int_ref		flight_log_max;
	int_ref		ignite_mode;
	string_ref	version;
	string_ref	product;
	string_ref	callsign;
	AltosConfigUI	config_ui;
	boolean		serial_started;

	boolean get_int(String line, String label, int_ref x) {
		if (line.startsWith(label)) {
			try {
				String tail = line.substring(label.length()).trim();
				String[] tokens = tail.split("\\s+");
				if (tokens.length > 0) {
					int	i = Integer.parseInt(tokens[0]);
					x.set(i);
					return true;
				}
			} catch (NumberFormatException ne) {
			}
		}
		return false;
	}

	boolean get_string(String line, String label, string_ref s) {
		if (line.startsWith(label)) {
			String	quoted = line.substring(label.length()).trim();

			if (quoted.startsWith("\""))
				quoted = quoted.substring(1);
			if (quoted.endsWith("\""))
				quoted = quoted.substring(0,quoted.length()-1);
			s.set(quoted);
			return true;
		} else {
			return false;
		}
	}

	void start_serial() throws InterruptedException {
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

	void update_ui() {
		config_ui.set_serial(serial.get());
		config_ui.set_product(product.get());
		config_ui.set_version(version.get());
		config_ui.set_main_deploy(main_deploy.get());
		config_ui.set_apogee_delay(apogee_delay.get());
		config_ui.set_radio_channel(radio_channel.get());
		config_ui.set_radio_calibration(radio_calibration.get());
		config_ui.set_flight_log_max(flight_log_max.get());
		config_ui.set_ignite_mode(ignite_mode.get());
		config_ui.set_callsign(callsign.get());
		config_ui.set_clean();
		config_ui.make_visible();
	}

	void process_line(String line) {
		if (line == null) {
			System.out.printf("timeout\n");
			abort();
			return;
		}
		if (line.equals("done")) {
			System.out.printf("done\n");
			if (serial_line != null)
				update_ui();
			return;
		}
		get_int(line, "serial-number", serial);
		get_int(line, "Main deploy:", main_deploy);
		get_int(line, "Apogee delay:", apogee_delay);
		get_int(line, "Radio channel:", radio_channel);
		get_int(line, "Radio cal:", radio_calibration);
		get_int(line, "Max flight log:", flight_log_max);
		get_int(line, "Ignite mode:", ignite_mode);
		get_string(line, "Callsign:", callsign);
		get_string(line,"software-version", version);
		get_string(line,"product", product);
	}

	final static int	serial_mode_read = 0;
	final static int	serial_mode_save = 1;
	final static int	serial_mode_reboot = 2;

	class SerialData implements Runnable {
		AltosConfig	config;
		int		serial_mode;

		void process_line(String line) {
			config.process_line(line);
		}
		void callback(String in_line) {
			final String line = in_line;
			Runnable r = new Runnable() {
					public void run() {
						process_line(line);
					}
				};
			SwingUtilities.invokeLater(r);
		}

		void get_data() {
			try {
				config.start_serial();
				config.serial_line.printf("c s\nv\n");
				for (;;) {
					try {
						String line = config.serial_line.get_reply(5000);
						if (line == null)
							stop_serial();
						callback(line);
						if (line.startsWith("software-version"))
							break;
					} catch (Exception e) {
						break;
					}
				}
			} catch (InterruptedException ie) {
			} finally {
				try {
					stop_serial();
				} catch (InterruptedException ie) {
				}
			}
			callback("done");
		}

		void save_data() {
			try {
				int	channel;
				start_serial();
				serial_line.printf("c m %d\n", main_deploy.get());
				serial_line.printf("c d %d\n", apogee_delay.get());
				channel = radio_channel.get();
				serial_line.printf("c r %d\n", channel);
				if (remote) {
					serial_line.stop_remote();
					serial_line.set_channel(channel);
					AltosPreferences.set_channel(device.getSerial(), channel);
					serial_line.start_remote();
				}
				if (!remote)
					serial_line.printf("c f %d\n", radio_calibration.get());
				serial_line.printf("c c %s\n", callsign.get());
				if (flight_log_max.get() != 0)
					serial_line.printf("c l %d\n", flight_log_max.get());
				if (ignite_mode.get() >= 0)
					serial_line.printf("c i %d\n", ignite_mode.get());
				serial_line.printf("c w\n");
			} catch (InterruptedException ie) {
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
		serial_line.close();
		serial_line = null;
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

	void save_data() {
		main_deploy.set(config_ui.main_deploy());
		apogee_delay.set(config_ui.apogee_delay());
		radio_channel.set(config_ui.radio_channel());
		radio_calibration.set(config_ui.radio_calibration());
		flight_log_max.set(config_ui.flight_log_max());
		ignite_mode.set(config_ui.ignite_mode());
		callsign.set(config_ui.callsign());
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

		serial = new int_ref(0);
		main_deploy = new int_ref(250);
		apogee_delay = new int_ref(0);
		radio_channel = new int_ref(0);
		radio_calibration = new int_ref(1186611);
		flight_log_max = new int_ref(0);
		ignite_mode = new int_ref(-1);
		callsign = new string_ref("N0CALL");
		version = new string_ref("unknown");
		product = new string_ref("unknown");

		device = AltosDeviceDialog.show(owner, Altos.product_any);
		if (device != null) {
			try {
				serial_line = new AltosSerial(device);
				if (!device.matchProduct(Altos.product_telemetrum))
					remote = true;
				try {
					init_ui();
				} catch (InterruptedException ie) {
					abort();
				} catch (TimeoutException te) {
					abort();
				}
			} catch (FileNotFoundException ee) {
				JOptionPane.showMessageDialog(owner,
							      String.format("Cannot open device \"%s\"",
									    device.toShortString()),
							      "Cannot open target device",
							      JOptionPane.ERROR_MESSAGE);
			} catch (AltosSerialInUseException si) {
				JOptionPane.showMessageDialog(owner,
							      String.format("Device \"%s\" already in use",
									    device.toShortString()),
							      "Device in use",
							      JOptionPane.ERROR_MESSAGE);
			} catch (IOException ee) {
				JOptionPane.showMessageDialog(owner,
							      device.toShortString(),
							      ee.getLocalizedMessage(),
							      JOptionPane.ERROR_MESSAGE);
			}
		}
	}
}
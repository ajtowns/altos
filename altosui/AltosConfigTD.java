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

import org.altusmetrum.AltosLib.*;

public class AltosConfigTD implements ActionListener {

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
	int_ref		serial;
	int_ref		radio_channel;
	int_ref		radio_calibration;
	int_ref		radio_setting;
	int_ref		radio_frequency;
	string_ref	config_version;
	string_ref	version;
	string_ref	product;
	AltosConfigTDUI	config_ui;
	boolean		serial_started;
	boolean		made_visible;

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

	void start_serial() throws InterruptedException, TimeoutException {
		serial_started = true;
	}

	void stop_serial() throws InterruptedException {
		if (!serial_started)
			return;
		serial_started = false;
	}

	void update_ui() {
		config_ui.set_serial(serial.get());
		config_ui.set_product(product.get());
		config_ui.set_version(version.get());
		config_ui.set_radio_frequency(frequency());
		config_ui.set_radio_calibration(radio_calibration.get());
		config_ui.set_clean();
		if (!made_visible) {
			made_visible = true;
			config_ui.make_visible();
		}
	}

	void process_line(String line) {
		if (line == null) {
			abort();
			return;
		}
		if (line.equals("all finished")) {
			if (serial_line != null)
				update_ui();
			return;
		}
		get_string(line, "Config version", config_version);
		get_int(line, "serial-number", serial);
		get_int(line, "Radio channel:", radio_channel);
		get_int(line, "Radio cal:", radio_calibration);
		get_int(line, "Frequency:", radio_frequency);
		get_int(line, "Radio setting:", radio_setting);
		get_string(line,"software-version", version);
		get_string(line,"product", product);
	}

	final static int	serial_mode_read = 0;
	final static int	serial_mode_save = 1;
	final static int	serial_mode_reboot = 2;

	class SerialData implements Runnable {
		AltosConfigTD	config;
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

		void reset_data() {
			serial.set(0);
			radio_channel.set(0);
			radio_setting.set(0);
			radio_frequency.set(0);
			radio_calibration.set(1186611);
			config_version.set("0.0");
			version.set("unknown");
			product.set("unknown");
		}

		void get_data() {
			try {
				boolean	been_there = false;
				config.start_serial();
				reset_data();

				for (;;) {
					config.serial_line.printf("c s\nf\nl\nv\n");
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
					if (been_there)
						break;
					if (!config_version.get().equals("0.0"))
						break;
					been_there = true;
					config.serial_line.printf("C\n ");
					config.serial_line.flush_input();
				}
			} catch (InterruptedException ie) {
			} catch (TimeoutException te) {
			} finally {
				try {
					stop_serial();
				} catch (InterruptedException ie) {
				}
			}
			double	pref_frequency = AltosPreferences.frequency(serial.get());
			if (pref_frequency != 0)
				radio_frequency.set((int) Math.floor (pref_frequency * 1000 + 0.5));
			callback("all finished");
		}

		void save_data() {
			double frequency = frequency();
			if (frequency != 0)
				AltosPreferences.set_frequency(serial.get(),
							       frequency);
		}

		public void run () {
			switch (serial_mode) {
			case serial_mode_save:
				save_data();
				/* fall through ... */
			case serial_mode_read:
				get_data();
				break;
			}
		}

		public SerialData(AltosConfigTD in_config, int in_serial_mode) {
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
		config_ui = new AltosConfigTDUI(owner);
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

	double frequency() {
		return AltosConvert.radio_to_frequency(radio_frequency.get(),
						       radio_setting.get(),
						       radio_calibration.get(),
						       radio_channel.get());
	}

	void set_frequency(double freq) {
		int	frequency = radio_frequency.get();
		int	setting = radio_setting.get();

		if (frequency > 0) {
			radio_frequency.set((int) Math.floor (freq * 1000 + 0.5));
		} else if (setting > 0) {
			radio_setting.set(AltosConvert.radio_frequency_to_setting(freq,
										  radio_calibration.get()));
			radio_channel.set(0);
		} else {
			radio_channel.set(AltosConvert.radio_frequency_to_channel(freq));
		}
	}

	void save_data() {

		set_frequency(config_ui.radio_frequency());
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

	public AltosConfigTD(JFrame given_owner) {
		owner = given_owner;

		serial = new int_ref(0);
		radio_channel = new int_ref(0);
		radio_setting = new int_ref(0);
		radio_frequency = new int_ref(0);
		radio_calibration = new int_ref(1186611);
		config_version = new string_ref("0.0");
		version = new string_ref("unknown");
		product = new string_ref("unknown");

		device = AltosDeviceDialog.show(owner, Altos.product_basestation);
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
			} catch (IOException ee) {
				JOptionPane.showMessageDialog(owner,
							      device.toShortString(),
							      ee.getLocalizedMessage(),
							      JOptionPane.ERROR_MESSAGE);
			}
		}
	}
}
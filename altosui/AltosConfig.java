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
import org.altusmetrum.AltosLib.*;

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
	AltosConfigData	remote_config_data;
	double		remote_frequency;
	int_ref		serial;
	int_ref		log_format;
	int_ref		main_deploy;
	int_ref		apogee_delay;
	int_ref		apogee_lockout;
	int_ref		radio_channel;
	int_ref		radio_calibration;
	int_ref		flight_log_max;
	int_ref		ignite_mode;
	int_ref		pad_orientation;
	int_ref		radio_setting;
	int_ref		radio_frequency;
	int_ref		storage_size;
	int_ref		storage_erase_unit;
	int_ref		stored_flight;
	int_ref		radio_enable;
	string_ref	version;
	string_ref	product;
	string_ref	callsign;
	AltosConfigUI	config_ui;
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
		if (storage_size.get() > 0 && storage_erase_unit.get() > 0) {
			int	log_limit = storage_size.get() - storage_erase_unit.get();
			if (log_limit > 0)
				return log_limit / 1024;
		}
		return 1024;
	}

	void update_ui() {
		config_ui.set_serial(serial.get());
		config_ui.set_product(product.get());
		config_ui.set_version(version.get());
		config_ui.set_main_deploy(main_deploy.get());
		config_ui.set_apogee_delay(apogee_delay.get());
		config_ui.set_apogee_lockout(apogee_lockout.get());
		config_ui.set_radio_calibration(radio_calibration.get());
		config_ui.set_radio_frequency(frequency());
		boolean max_enabled = true;
		switch (log_format.get()) {
		case Altos.AO_LOG_FORMAT_TINY:
			max_enabled = false;
			break;
		default:
			if (stored_flight.get() >= 0)
				max_enabled = false;
			break;
		}
		config_ui.set_flight_log_max_enabled(max_enabled);
		config_ui.set_radio_enable(radio_enable.get());
		config_ui.set_flight_log_max_limit(log_limit());
		config_ui.set_flight_log_max(flight_log_max.get());
		config_ui.set_ignite_mode(ignite_mode.get());
		config_ui.set_pad_orientation(pad_orientation.get());
		config_ui.set_callsign(callsign.get());
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
		get_int(line, "serial-number", serial);
		get_int(line, "log-format", log_format);
		get_int(line, "Main deploy:", main_deploy);
		get_int(line, "Apogee delay:", apogee_delay);
		get_int(line, "Apogee lockout:", apogee_lockout);
		get_int(line, "Radio channel:", radio_channel);
		get_int(line, "Radio cal:", radio_calibration);
		get_int(line, "Max flight log:", flight_log_max);
		get_int(line, "Ignite mode:", ignite_mode);
		get_int(line, "Pad orientation:", pad_orientation);
		get_int(line, "Radio setting:", radio_setting);
		if (get_int(line, "Frequency:", radio_frequency))
			if (radio_frequency.get() < 0)
				radio_frequency.set(434550);
		get_int(line, "Radio enable:", radio_enable);
		get_int(line, "Storage size:", storage_size);
		get_int(line, "Storage erase unit:", storage_erase_unit);
		get_int(line, "flight", stored_flight);
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

		void reset_data() {
			serial.set(0);
			log_format.set(Altos.AO_LOG_FORMAT_UNKNOWN);
			main_deploy.set(250);
			apogee_delay.set(0);
			apogee_lockout.set(0);
			radio_channel.set(0);
			radio_setting.set(0);
			radio_frequency.set(0);
			radio_calibration.set(1186611);
			radio_enable.set(-1);
			flight_log_max.set(0);
			ignite_mode.set(-1);
			pad_orientation.set(-1);
			storage_size.set(-1);
			storage_erase_unit.set(-1);
			stored_flight.set(-1);
			callsign.set("N0CALL");
			version.set("unknown");
			product.set("unknown");
		}

		void get_data() {
			try {
				config.start_serial();
				reset_data();

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
			} catch (InterruptedException ie) {
			} catch (TimeoutException te) {
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
				boolean has_frequency = radio_frequency.get() > 0;
				boolean has_setting = radio_setting.get() > 0;
				start_serial();
				serial_line.printf("c m %d\n", main_deploy.get());
				serial_line.printf("c d %d\n", apogee_delay.get());
				serial_line.printf("c L %d\n", apogee_lockout.get());
				if (!remote)
					serial_line.printf("c f %d\n", radio_calibration.get());
				serial_line.set_radio_frequency(frequency,
								has_frequency,
								has_setting,
								radio_calibration.get());
				if (remote) {
					serial_line.stop_remote();
					serial_line.set_radio_frequency(frequency);
					AltosUIPreferences.set_frequency(device.getSerial(), frequency);
					serial_line.start_remote();
				}
				serial_line.printf("c c %s\n", callsign.get());
				if (flight_log_max.get() != 0)
					serial_line.printf("c l %d\n", flight_log_max.get());
				if (radio_enable.get() >= 0)
					serial_line.printf("c e %d\n", radio_enable.get());
				if (ignite_mode.get() >= 0)
					serial_line.printf("c i %d\n", ignite_mode.get());
				if (pad_orientation.get() >= 0)
					serial_line.printf("c o %d\n", pad_orientation.get());
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
			radio_channel.set(0);
		} else if (setting > 0) {
			radio_setting.set(AltosConvert.radio_frequency_to_setting(freq,
										  radio_calibration.get()));
			radio_channel.set(0);
		} else {
			radio_channel.set(AltosConvert.radio_frequency_to_channel(freq));
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

		main_deploy.set(config_ui.main_deploy());
		apogee_delay.set(config_ui.apogee_delay());
		apogee_lockout.set(config_ui.apogee_lockout());
		radio_calibration.set(config_ui.radio_calibration());
		set_frequency(config_ui.radio_frequency());
		flight_log_max.set(config_ui.flight_log_max());
		if (radio_enable.get() >= 0)
			radio_enable.set(config_ui.radio_enable());
		if (ignite_mode.get() >= 0)
			ignite_mode.set(config_ui.ignite_mode());
		if (pad_orientation.get() >= 0)
			pad_orientation.set(config_ui.pad_orientation());
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
		log_format = new int_ref(Altos.AO_LOG_FORMAT_UNKNOWN);
		main_deploy = new int_ref(250);
		apogee_delay = new int_ref(0);
		apogee_lockout = new int_ref(0);
		radio_channel = new int_ref(0);
		radio_setting = new int_ref(0);
		radio_frequency = new int_ref(0);
		radio_calibration = new int_ref(1186611);
		radio_enable = new int_ref(-1);
		flight_log_max = new int_ref(0);
		ignite_mode = new int_ref(-1);
		pad_orientation = new int_ref(-1);
		storage_size = new int_ref(-1);
		storage_erase_unit = new int_ref(-1);
		stored_flight = new int_ref(-1);
		callsign = new string_ref("N0CALL");
		version = new string_ref("unknown");
		product = new string_ref("unknown");

		device = AltosDeviceDialog.show(owner, Altos.product_any);
		if (device != null) {
			try {
				serial_line = new AltosSerial(device);
				try {
					if (!device.matchProduct(Altos.product_altimeter))
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
			} catch (IOException ee) {
				JOptionPane.showMessageDialog(owner,
							      device.toShortString(),
							      ee.getLocalizedMessage(),
							      JOptionPane.ERROR_MESSAGE);
			}
		}
	}
}
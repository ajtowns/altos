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

import altosui.Altos;
import altosui.AltosSerial;
import altosui.AltosSerialMonitor;
import altosui.AltosRecord;
import altosui.AltosTelemetry;
import altosui.AltosState;
import altosui.AltosDeviceDialog;
import altosui.AltosPreferences;
import altosui.AltosLog;
import altosui.AltosVoice;
import altosui.AltosFlightStatusTableModel;
import altosui.AltosFlightInfoTableModel;
import altosui.AltosConfigUI;

import libaltosJNI.*;

public class AltosConfig implements Runnable, ActionListener {

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
	Thread		config_thread;
	int_ref		serial;
	int_ref		main_deploy;
	int_ref		apogee_delay;
	int_ref		radio_channel;
	string_ref	version;
	string_ref	product;
	string_ref	callsign;
	AltosConfigUI	config_ui;


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
		if (remote) {
			serial_line.set_channel(AltosPreferences.channel());
			serial_line.set_callsign(AltosPreferences.callsign());
			serial_line.printf("p\n");
			serial_line.flush_input();
		}
	}

	void stop_serial() throws InterruptedException {
		if (remote) {
			serial_line.printf("~");
			serial_line.flush_output();
		}
	}

	void get_data() throws InterruptedException {
		try {
			start_serial();
			serial_line.printf("c s\nv\n");
			for (;;) {
				String line = serial_line.get_reply();
				get_int(line, "serial-number", serial);
				get_int(line, "Main deploy:", main_deploy);
				get_int(line, "Apogee delay:", apogee_delay);
				get_int(line, "Radio channel:", radio_channel);
				get_string(line, "Callsign:", callsign);
				get_string(line,"software-version", version);
				get_string(line,"product", product);

				/* signals the end of the version info */
				if (line.startsWith("software-version"))
					break;
			}
		} finally {
			stop_serial();
		}
	}

	void init_ui () {
		config_ui = new AltosConfigUI(owner);
		config_ui.addActionListener(this);
		set_ui();
	}

	void set_ui() {
		try {
			if (serial_line != null)
				get_data();
			config_ui.set_serial(serial.get());
			config_ui.set_product(product.get());
			config_ui.set_version(version.get());
			config_ui.set_main_deploy(main_deploy.get());
			config_ui.set_apogee_delay(apogee_delay.get());
			config_ui.set_radio_channel(radio_channel.get());
			config_ui.set_callsign(callsign.get());
			config_ui.set_clean();
		} catch (InterruptedException ie) {
		}
	}

	void run_dialog() {
	}

	void save_data() {
		main_deploy.set(config_ui.main_deploy());
		apogee_delay.set(config_ui.apogee_delay());
		radio_channel.set(config_ui.radio_channel());
		callsign.set(config_ui.callsign());
		try {
			start_serial();
			serial_line.printf("c m %d\n", main_deploy.get());
			serial_line.printf("c d %d\n", apogee_delay.get());
			serial_line.printf("c r %d\n", radio_channel.get());
			serial_line.printf("c c %s\n", callsign.get());
			serial_line.printf("c w\n");
		} catch (InterruptedException ie) {
		} finally {
			try {
				stop_serial();
			} catch (InterruptedException ie) {
			}
		}
	}

	public void actionPerformed(ActionEvent e) {
		String	cmd = e.getActionCommand();
		if (cmd.equals("save")) {
			save_data();
			set_ui();
		} else if (cmd.equals("reset")) {
			set_ui();
		} else if (cmd.equals("close")) {
			if (serial_line != null)
				serial_line.close();
		}
	}

	public void run () {
		try {
			init_ui();
			config_ui.make_visible();
//		} catch (InterruptedException ie) {
		} finally {
		}
	}

	public AltosConfig(JFrame given_owner) {
		owner = given_owner;

		serial = new int_ref(0);
		main_deploy = new int_ref(250);
		apogee_delay = new int_ref(0);
		radio_channel = new int_ref(0);
		callsign = new string_ref("N0CALL");
		version = new string_ref("unknown");
		product = new string_ref("unknown");

		device = AltosDeviceDialog.show(owner, AltosDevice.product_any);
		serial_line = new AltosSerial();
		if (device != null) {
			try {
				serial_line.open(device);
				if (!device.matchProduct(AltosDevice.product_telemetrum))
					remote = true;
				config_thread = new Thread(this);
				config_thread.start();
			} catch (FileNotFoundException ee) {
				JOptionPane.showMessageDialog(owner,
							      String.format("Cannot open device \"%s\"",
									    device.getPath()),
							      "Cannot open target device",
							      JOptionPane.ERROR_MESSAGE);
			} catch (IOException ee) {
				JOptionPane.showMessageDialog(owner,
							      device.getPath(),
							      ee.getLocalizedMessage(),
							      JOptionPane.ERROR_MESSAGE);
			}
		}
	}
}
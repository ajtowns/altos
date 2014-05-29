/*
 * Copyright © 2014 Keith Packard <keithp@keithp.com>
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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.io.*;
import java.util.concurrent.*;
import java.util.*;
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

public class TeleGPS extends AltosUIFrame implements AltosFlightDisplay, AltosFontListener, ActionListener {

	static String[] telegps_icon_names = {
		"/telegps-16.png",
		"/telegps-32.png",
		"/telegps-48.png",
		"/telegps-64.png",
		"/telegps-128.png",
		"/telegps-256.png"
	};

	static { set_icon_names(telegps_icon_names); }

	static AltosVoice	voice;

	static AltosVoice voice() {
		if (voice == null)
			voice = new AltosVoice();
		return voice;
	}

	AltosFlightReader	reader;
	AltosDisplayThread	thread;

	JTabbedPane	pane;

	AltosSiteMap    sitemap;
	boolean		has_map;

	JMenuBar	menu_bar;
	JMenu		file_menu;
	JMenu		monitor_menu;
	JMenu		device_menu;

	/* File menu */
	final static String	new_command = "new";
	final static String	preferences_command = "preferences";
	final static String	load_maps_command = "loadmaps";
	final static String	close_command = "close";
	final static String	exit_command = "exit";

	static final String[][] file_menu_entries = new String[][] {
		{ "New Window",		new_command },
		{ "Preferences",	preferences_command },
		{ "Load Maps",		load_maps_command },
		{ "Close",		close_command },
		{ "Exit",		exit_command },
	};

	/* Monitor menu */
	final static String	monitor_command = "monitor";
	final static String	disconnect_command = "disconnect";
	final static String	scan_command = "scan";

	static final String[][] monitor_menu_entries = new String[][] {
		{ "Monitor Device",	monitor_command },
		{ "Disconnect",		disconnect_command },
		{ "Scan Channels",	scan_command },
	};

	/* Device menu */
	final static String	download_command = "download";
	final static String	configure_command = "configure";
	final static String	export_command = "export";
	final static String	graph_command = "graph";

	static final String[][] device_menu_entries = new String[][] {
		{ "Download Data",	download_command },
		{ "Configure Device",	configure_command },
		{ "Export Data",	export_command },
		{ "Graph Data",		graph_command },
	};

//	private AltosInfoTable flightInfo;

	boolean exit_on_close = false;

	void stop_display() {
		if (thread != null && thread.isAlive()) {
			thread.interrupt();
			try {
				thread.join();
			} catch (InterruptedException ie) {}
		}
		thread = null;
	}

	public void reset() {
		sitemap.reset();
	}

	public void set_font() {
		sitemap.set_font();
	}

	public void font_size_changed(int font_size) {
		set_font();
	}


//	AltosFlightStatusUpdate	status_update;

	public void show(AltosState state, AltosListenerState listener_state) {
//		status_update.saved_state = state;

		if (state == null)
			state = new AltosState();

		sitemap.show(state, listener_state);
		telegps_status.show(state, listener_state);
	}

	Container		bag;
	AltosFreqList		frequencies;
	JLabel			telemetry;
	TeleGPSStatus		telegps_status;
	TeleGPSStatusUpdate	status_update;

	ActionListener		show_timer;

	void new_window() {
		new TeleGPS();
	}

	void preferences() {
		new TeleGPSPreferences(this, voice());
	}

	void load_maps() {
		new AltosSiteMapPreload(this);
	}

	void disconnect() {
		setTitle("TeleGPS");
		stop_display();
		remove_frequency_menu();
	}

	void connect(AltosDevice device) {
		if (reader != null)
			disconnect();
		try {
			AltosFlightReader	reader = new AltosTelemetryReader(new AltosSerial(device));
			set_reader(reader);
			add_frequency_menu(device.getSerial(), reader);
		} catch (FileNotFoundException ee) {
			JOptionPane.showMessageDialog(this,
						      ee.getMessage(),
						      String.format ("Cannot open %s", device.toShortString()),
						      JOptionPane.ERROR_MESSAGE);
		} catch (AltosSerialInUseException si) {
			JOptionPane.showMessageDialog(this,
						      String.format("Device \"%s\" already in use",
								    device.toShortString()),
						      "Device in use",
						      JOptionPane.ERROR_MESSAGE);
		} catch (IOException ee) {
			JOptionPane.showMessageDialog(this,
						      String.format ("Unknown I/O error on %s", device.toShortString()),
						      "Unknown I/O error",
						      JOptionPane.ERROR_MESSAGE);
		} catch (TimeoutException te) {
			JOptionPane.showMessageDialog(this,
						      String.format ("Timeout on %s", device.toShortString()),
						      "Timeout error",
						      JOptionPane.ERROR_MESSAGE);
		} catch (InterruptedException ie) {
			JOptionPane.showMessageDialog(this,
						      String.format("Interrupted %s", device.toShortString()),
						      "Interrupted exception",
						      JOptionPane.ERROR_MESSAGE);
		}
	}

	void monitor() {
		AltosDevice	device = AltosDeviceUIDialog.show(this,
								  AltosLib.product_basestation);
		if (device == null)
			return;
		connect(device);
	}

	public void scan_device_selected(AltosDevice device) {
		connect(device);
	}

	void scan() {
		new AltosScanUI(this, false);
	}

	void download(){
		new AltosEepromManage(this, AltosLib.product_telegps);
	}

	void configure() {
		new TeleGPSConfig(this);
	}

	void export() {
	}

	void graph() {
	}

	public void actionPerformed(ActionEvent ev) {

		/* File menu */
		if (new_command.equals(ev.getActionCommand())) {
			new_window();
			return;
		}
		if (preferences_command.equals(ev.getActionCommand())) {
			preferences();
			return;
		}
		if (load_maps_command.equals(ev.getActionCommand())) {
			load_maps();
			return;
		}
		if (close_command.equals(ev.getActionCommand())) {
			close();
			return;
		}
		if (exit_command.equals(ev.getActionCommand()))
			System.exit(0);

		/* Monitor menu */
		if (monitor_command.equals(ev.getActionCommand())) {
			monitor();
			return;
		}
		if (disconnect_command.equals(ev.getActionCommand())) {
			disconnect();
			return;
		}
		if (scan_command.equals(ev.getActionCommand())) {
			scan();
			return;
		}

		/* Device menu */
		if (download_command.equals(ev.getActionCommand())) {
			download();
			return;
		}
		if (configure_command.equals(ev.getActionCommand())) {
			configure();
			return;
		}
		if (export_command.equals(ev.getActionCommand())) {
			export();
			return;
		}
		if (graph_command.equals(ev.getActionCommand())) {
			graph();
			return;
		}
	}

	void add_frequency_menu(int serial, final AltosFlightReader reader) {
		// Channel menu
		frequencies = new AltosFreqList(AltosUIPreferences.frequency(serial));
		frequencies.set_product("Monitor");
		frequencies.set_serial(serial);
		frequencies.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					double frequency = frequencies.frequency();
					try {
						reader.set_frequency(frequency);
					} catch (TimeoutException te) {
					} catch (InterruptedException ie) {
					}
					reader.save_frequency();
				}
			});
		menu_bar.add(frequencies);
	}

	void remove_frequency_menu() {
		if (frequencies != null) {
			menu_bar.remove(frequencies);
			frequencies = null;
		}
	}

	public void set_reader(AltosFlightReader reader) {
		setTitle(String.format("TeleGPS %s", reader.name));
		thread = new AltosDisplayThread(this, voice(), this, reader);
		thread.start();
	}

	static int	number_of_windows;

	private void close() {
		AltosUIPreferences.unregister_font_listener(this);
		setVisible(false);
		dispose();
		--number_of_windows;
		if (number_of_windows == 0)
			System.exit(0);
	}

	private void add_menu(JMenu menu, String label, String action) {
		JMenuItem	item = new JMenuItem(label);
		menu.add(item);
		item.addActionListener(this);
		item.setActionCommand(action);
	}


	private JMenu make_menu(String label, String[][] items) {
		JMenu	menu = new JMenu(label);
		for (int i = 0; i < items.length; i++)
			add_menu(menu, items[i][0], items[i][1]);
		menu_bar.add(menu);
		return menu;
	}

	public TeleGPS() {

		AltosUIPreferences.set_component(this);

		reader = null;

		bag = getContentPane();
		bag.setLayout(new GridBagLayout());

		GridBagConstraints c = new GridBagConstraints();

		setTitle("TeleGPS");

		menu_bar = new JMenuBar();
		setJMenuBar(menu_bar);

		file_menu = make_menu("File", file_menu_entries);
		monitor_menu = make_menu("Monitor", monitor_menu_entries);
		device_menu = make_menu("Device", device_menu_entries);

		int serial = -1;

		/* TeleGPS status is always visible */
		telegps_status = new TeleGPSStatus();
		c.gridx = 0;
		c.gridy = 1;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.gridwidth = 2;
		bag.add(telegps_status, c);
		c.gridwidth = 1;


		/* The rest of the window uses a tabbed pane to
		 * show one of the alternate data views
		 */
		pane = new JTabbedPane();

		/* Make the tabbed pane use the rest of the window space */
		c.gridx = 0;
		c.gridy = 2;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
		c.weighty = 1;
		c.gridwidth = 2;
		bag.add(pane, c);

		sitemap = new AltosSiteMap();
		pane.add("Site Map", sitemap);

		setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);

		AltosUIPreferences.register_font_listener(this);

		addWindowListener(new WindowAdapter() {
				@Override
				public void windowClosing(WindowEvent e) {
					close();
				}
			});

		pack();
		setVisible(true);

		++number_of_windows;

		status_update = new TeleGPSStatusUpdate(telegps_status);

		new javax.swing.Timer(100, status_update).start();
	}

	public TeleGPS(AltosFlightReader reader) {
		this();
		set_reader(reader);
	}

	static AltosStateIterable record_iterable(File file) {
		FileInputStream in;
		try {
			in = new FileInputStream(file);
		} catch (Exception e) {
			System.out.printf("Failed to open file '%s'\n", file);
			return null;
		}
		if (file.getName().endsWith("telem"))
			return new AltosTelemetryFile(in);
		else
			return new AltosEepromFile(in);
	}

	static AltosReplayReader replay_file(File file) {
		AltosStateIterable states = record_iterable(file);
		if (states == null)
			return null;
		return new AltosReplayReader(states.iterator(), file);
	}

	static boolean process_replay(File file) {
		AltosReplayReader new_reader = replay_file(file);
		if (new_reader == null)
			return false;

		new TeleGPS(new_reader);
		return true;
	}

	static final int process_none = 0;
	static final int process_csv = 1;
	static final int process_kml = 2;
	static final int process_graph = 3;
	static final int process_replay = 4;
	static final int process_summary = 5;
	static final int process_cat = 6;

	public static boolean load_library(Frame frame) {
		if (!AltosUILib.load_library()) {
			JOptionPane.showMessageDialog(frame,
						      String.format("No AltOS library in \"%s\"",
								    System.getProperty("java.library.path","<undefined>")),
						      "Cannot load device access library",
						      JOptionPane.ERROR_MESSAGE);
			return false;
		}
		return true;
	}

	public static void help(int code) {
		System.out.printf("Usage: altosui [OPTION]... [FILE]...\n");
		System.out.printf("  Options:\n");
		System.out.printf("    --fetchmaps <lat> <lon>\tpre-fetch maps for site map view\n");
		System.out.printf("    --replay <filename>\t\trelive the glory of past flights \n");
		System.out.printf("    --graph <filename>\t\tgraph a flight\n");
		System.out.printf("    --csv\tgenerate comma separated output for spreadsheets, etc\n");
		System.out.printf("    --kml\tgenerate KML output for use with Google Earth\n");
		System.exit(code);
	}

	public static void main(String[] args) {
		int	errors = 0;

		load_library(null);
		try {
			UIManager.setLookAndFeel(AltosUIPreferences.look_and_feel());
		} catch (Exception e) {
		}

		boolean	any_created = false;


		/* Handle batch-mode */
		int process = process_none;
		for (int i = 0; i < args.length; i++) {
			if (args[i].equals("--help"))
				help(0);
			else if (args[i].equals("--fetchmaps")) {
				if (args.length < i + 3) {
					help(1);
				} else {
					double lat = Double.parseDouble(args[i+1]);
					double lon = Double.parseDouble(args[i+2]);
					AltosSiteMap.prefetchMaps(lat, lon);
					i += 2;
				}
			} else if (args[i].equals("--replay"))
				process = process_replay;
			else if (args[i].equals("--kml"))
				process = process_kml;
			else if (args[i].equals("--csv"))
				process = process_csv;
			else if (args[i].equals("--graph"))
				process = process_graph;
			else if (args[i].equals("--summary"))
				process = process_summary;
			else if (args[i].equals("--cat"))
				process = process_cat;
			else if (args[i].startsWith("--"))
				help(1);
			else {
				File file = new File(args[i]);
				switch (process) {
				case process_graph:
					++errors;
					break;
				case process_none:
				case process_replay:
					if (!process_replay(file))
						++errors;
					any_created = true;
					break;
				case process_kml:
					++errors;
					break;
				case process_csv:
					++errors;
					break;
				case process_summary:
					++errors;
					break;
				case process_cat:
					++errors;
				}
			}
		}
		if (errors != 0)
			System.exit(errors);
		if (!any_created)
			new TeleGPS();
	}
}

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

public class AltosUI extends AltosFrame {
	public AltosVoice voice = new AltosVoice();

	public static boolean load_library(Frame frame) {
		if (!Altos.load_library()) {
			JOptionPane.showMessageDialog(frame,
						      String.format("No AltOS library in \"%s\"",
								    System.getProperty("java.library.path","<undefined>")),
						      "Cannot load device access library",
						      JOptionPane.ERROR_MESSAGE);
			return false;
		}
		return true;
	}

	void telemetry_window(AltosDevice device) {
		try {
			AltosFlightReader reader = new AltosTelemetryReader(new AltosSerial(device));
			if (reader != null)
				new AltosFlightUI(voice, reader, device.getSerial());
		} catch (FileNotFoundException ee) {
			JOptionPane.showMessageDialog(AltosUI.this,
						      ee.getMessage(),
						      "Cannot open target device",
						      JOptionPane.ERROR_MESSAGE);
		} catch (AltosSerialInUseException si) {
			JOptionPane.showMessageDialog(AltosUI.this,
						      String.format("Device \"%s\" already in use",
								    device.toShortString()),
						      "Device in use",
						      JOptionPane.ERROR_MESSAGE);
		} catch (IOException ee) {
			JOptionPane.showMessageDialog(AltosUI.this,
						      device.toShortString(),
						      "Unkonwn I/O error",
						      JOptionPane.ERROR_MESSAGE);
		} catch (TimeoutException te) {
			JOptionPane.showMessageDialog(this,
						      device.toShortString(),
						      "Timeout error",
						      JOptionPane.ERROR_MESSAGE);
		} catch (InterruptedException ie) {
			JOptionPane.showMessageDialog(this,
						      device.toShortString(),
						      "Interrupted exception",
						      JOptionPane.ERROR_MESSAGE);
		}
	}

	Container	pane;
	GridBagLayout	gridbag;

	JButton addButton(int x, int y, String label) {
		GridBagConstraints	c;
		JButton			b;

		c = new GridBagConstraints();
		c.gridx = x; c.gridy = y;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
		c.weighty = 1;
		b = new JButton(label);

		Dimension ps = b.getPreferredSize();

		gridbag.setConstraints(b, c);
		add(b, c);
		return b;
	}

	public AltosUI() {

		load_library(null);

		AltosUIPreferences.set_component(this);

		pane = getContentPane();
		gridbag = new GridBagLayout();
		pane.setLayout(gridbag);

		JButton	b;

		b = addButton(0, 0, "Monitor Flight");
		b.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						ConnectToDevice();
					}
				});
		b.setToolTipText("Connect to TeleDongle and monitor telemetry");
		b = addButton(1, 0, "Save Flight Data");
		b.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						SaveFlightData();
					}
				});
		b.setToolTipText("Download and/or delete flight data from an altimeter");
		b = addButton(2, 0, "Replay Flight");
		b.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						Replay();
					}
				});
		b.setToolTipText("Watch an old flight in real-time");
		b = addButton(3, 0, "Graph Data");
		b.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						GraphData();
					}
				});
		b.setToolTipText("Present flight data in a graph and table of statistics");
		b = addButton(4, 0, "Export Data");
		b.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						ExportData();
					}
				});
		b.setToolTipText("Convert flight data for a spreadsheet or GoogleEarth");
		b = addButton(0, 1, "Configure Altimeter");
		b.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						ConfigureTeleMetrum();
					}
				});
		b.setToolTipText("Set flight, storage and communication parameters");
		b = addButton(1, 1, "Configure AltosUI");
		b.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					ConfigureAltosUI();
				}
			});
		b.setToolTipText("Global AltosUI settings");

		b = addButton(2, 1, "Configure Ground Station");
		b.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					ConfigureTeleDongle();
				}
			});

		b = addButton(3, 1, "Flash Image");
		b.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					FlashImage();
				}
			});
		b.setToolTipText("Replace the firmware in any AltusMetrum product");

		b = addButton(4, 1, "Fire Igniter");
		b.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					FireIgniter();
				}
			});
		b.setToolTipText("Remote control of igniters for deployment testing");
		b = addButton(0, 2, "Scan Channels");
		b.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					ScanChannels();
				}
			});
		b.setToolTipText("Find what channel an altimeter is sending telemetry on");
		b = addButton(1, 2, "Load Maps");
		b.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					LoadMaps();
				}
			});
		b.setToolTipText("Download satellite images for off-line flight monitoring");
		b = addButton(2, 2, "Monitor Idle");
		b.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					IdleMonitor();
				}
			});
		b.setToolTipText("Check flight readiness of altimeter in idle mode");

//		b = addButton(3, 2, "Launch Controller");
//		b.addActionListener(new ActionListener() {
//				public void actionPerformed(ActionEvent e) {
//					LaunchController();
//				}
//			});

		b = addButton(4, 2, "Quit");
		b.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					System.exit(0);
				}
			});
		b.setToolTipText("Close all active windows and terminate AltosUI");

		setTitle("AltOS");

		pane.doLayout();
		pane.validate();

		doLayout();
		validate();

		setVisible(true);

		Insets i = getInsets();
		Dimension ps = rootPane.getPreferredSize();
		ps.width += i.left + i.right;
		ps.height += i.top + i.bottom;
		setPreferredSize(ps);
		setSize(ps);
		setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
		addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosing(WindowEvent e) {
				System.exit(0);
			}
		});
	}

	private void ConnectToDevice() {
		AltosDevice	device = AltosDeviceDialog.show(AltosUI.this,
								Altos.product_basestation);

		if (device != null)
			telemetry_window(device);
	}

	void ConfigureCallsign() {
		String	result;
		result = JOptionPane.showInputDialog(AltosUI.this,
						     "Configure Callsign",
						     AltosUIPreferences.callsign());
		if (result != null)
			AltosUIPreferences.set_callsign(result);
	}

	void ConfigureTeleMetrum() {
		new AltosConfig(AltosUI.this);
	}

	void ConfigureTeleDongle() {
		new AltosConfigTD(AltosUI.this);
	}

	void FlashImage() {
		AltosFlashUI.show(AltosUI.this);
	}

	void FireIgniter() {
		new AltosIgniteUI(AltosUI.this);
	}

	void ScanChannels() {
		new AltosScanUI(AltosUI.this);
	}

	void LoadMaps() {
		new AltosSiteMapPreload(AltosUI.this);
	}

	void LaunchController() {
		new AltosLaunchUI(AltosUI.this);
	}

	/*
	 * Replay a flight from telemetry data
	 */
	private void Replay() {
		AltosDataChooser chooser = new AltosDataChooser(
			AltosUI.this);

		AltosRecordIterable iterable = chooser.runDialog();
		if (iterable != null) {
			AltosFlightReader reader = new AltosReplayReader(iterable.iterator(),
									 chooser.file());
			new AltosFlightUI(voice, reader);
		}
	}

	/* Connect to TeleMetrum, either directly or through
	 * a TeleDongle over the packet link
	 */
	private void SaveFlightData() {
		new AltosEepromManage(AltosUI.this);
	}

	/* Load a flight log file and write out a CSV file containing
	 * all of the data in standard units
	 */

	private void ExportData() {
		AltosDataChooser chooser;
		chooser = new AltosDataChooser(this);
		AltosRecordIterable record_reader = chooser.runDialog();
		if (record_reader == null)
			return;
		new AltosCSVUI(AltosUI.this, record_reader, chooser.file());
	}

	/* Load a flight log CSV file and display a pretty graph.
	 */

	private void GraphData() {
		AltosDataChooser chooser;
		chooser = new AltosDataChooser(this);
		AltosRecordIterable record_reader = chooser.runDialog();
		if (record_reader == null)
			return;
		try {
			new AltosGraphUI(record_reader, chooser.filename());
		} catch (InterruptedException ie) {
		} catch (IOException ie) {
		}
	}

	private void ConfigureAltosUI() {
		new AltosConfigureUI(AltosUI.this, voice);
	}

	private void IdleMonitor() {
		try {
			new AltosIdleMonitorUI(this);
		} catch (Exception e) {
		}
	}

	static AltosRecordIterable open_logfile(String filename) {
		File file = new File (filename);
		try {
			FileInputStream in;

			in = new FileInputStream(file);
			if (filename.endsWith("eeprom"))
				return new AltosEepromIterable(in);
			else if (filename.endsWith("mega"))
				return new AltosEepromMegaIterable(in);
			else
				return new AltosTelemetryIterable(in);
		} catch (FileNotFoundException fe) {
			System.out.printf("%s\n", fe.getMessage());
			return null;
		}
	}

	static AltosWriter open_csv(String filename) {
		File file = new File (filename);
		try {
			return new AltosCSV(file);
		} catch (FileNotFoundException fe) {
			System.out.printf("%s\n", fe.getMessage());
			return null;
		}
	}

	static AltosWriter open_kml(String filename) {
		File file = new File (filename);
		try {
			return new AltosKML(file);
		} catch (FileNotFoundException fe) {
			System.out.printf("%s\n", fe.getMessage());
			return null;
		}
	}

	static final int process_none = 0;
	static final int process_csv = 1;
	static final int process_kml = 2;
	static final int process_graph = 3;
	static final int process_replay = 4;
	static final int process_summary = 5;

	static void process_csv(String input) {
		AltosRecordIterable iterable = open_logfile(input);
		if (iterable == null)
			return;

		String output = Altos.replace_extension(input,".csv");
		System.out.printf("Processing \"%s\" to \"%s\"\n", input, output);
		if (input.equals(output)) {
			System.out.printf("Not processing '%s'\n", input);
		} else {
			AltosWriter writer = open_csv(output);
			if (writer == null)
				return;
			writer.write(iterable);
			writer.close();
		}
	}

	static void process_kml(String input) {
		AltosRecordIterable iterable = open_logfile(input);
		if (iterable == null)
			return;

		String output = Altos.replace_extension(input,".kml");
		System.out.printf("Processing \"%s\" to \"%s\"\n", input, output);
		if (input.equals(output)) {
			System.out.printf("Not processing '%s'\n", input);
		} else {
			AltosWriter writer = open_kml(output);
			if (writer == null)
				return;
			writer.write(iterable);
			writer.close();
		}
	}

	static AltosRecordIterable record_iterable(File file) {
		FileInputStream in;
		try {
			in = new FileInputStream(file);
		} catch (Exception e) {
			System.out.printf("Failed to open file '%s'\n", file);
			return null;
		}
		AltosRecordIterable recs;
		AltosReplayReader reader;
		if (file.getName().endsWith("eeprom")) {
			recs = new AltosEepromIterable(in);
		} else {
			recs = new AltosTelemetryIterable(in);
		}
		return recs;
	}

	static AltosRecordIterable record_iterable_file(String filename) {
		return record_iterable (new File(filename));
	}

	static AltosReplayReader replay_file(String filename) {
		AltosRecordIterable recs = record_iterable_file(filename);
		if (recs == null)
			return null;
		return new AltosReplayReader(recs.iterator(), new File(filename));
	}

	static void process_replay(String filename) {
		AltosReplayReader reader = replay_file(filename);
		AltosFlightUI flight_ui = new AltosFlightUI(new AltosVoice(), reader);
		flight_ui.set_exit_on_close();
	}

	static void process_graph(String filename) {
		AltosRecordIterable recs = record_iterable_file(filename);
		if (recs == null)
			return;
		try {
			new AltosGraphUI(recs, filename);
		} catch (InterruptedException ie) {
		} catch (IOException ie) {
		}
	}
	
	static void process_summary(String filename) {
		AltosRecordIterable iterable = record_iterable_file(filename);
		try {
			AltosFlightStats stats = new AltosFlightStats(iterable);
			if (stats.serial > 0)
				System.out.printf("Serial:       %5d\n", stats.serial);
			if (stats.flight > 0)
				System.out.printf("Flight:       %5d\n", stats.flight);
			if (stats.year > 0)
				System.out.printf("Date:    %04d-%02d-%02d\n",
						  stats.year, stats.month, stats.day);
			if (stats.hour > 0)
				System.out.printf("Time:      %02d:%02d:%02d UTC\n",
						  stats.hour, stats.minute, stats.second);
			System.out.printf("Max height:  %6.0f m    %6.0f ft\n",
					  stats.max_height,
					  AltosConvert.meters_to_feet(stats.max_height));
			System.out.printf("Max speed:   %6.0f m/s  %6.0f ft/s  %6.4f Mach\n",
					  stats.max_speed,
					  AltosConvert.meters_to_feet(stats.max_speed),
					  AltosConvert.meters_to_mach(stats.max_speed));
			if (stats.max_acceleration != AltosRecord.MISSING) {
				System.out.printf("Max accel:   %6.0f m/s² %6.0f ft/s² %6.2f g\n",
						  stats.max_acceleration,
						  AltosConvert.meters_to_feet(stats.max_acceleration),
						  AltosConvert.meters_to_g(stats.max_acceleration));
			}
			System.out.printf("Drogue rate: %6.0f m/s  %6.0f ft/s\n",
					  stats.state_baro_speed[Altos.ao_flight_drogue],
					  AltosConvert.meters_to_feet(stats.state_baro_speed[Altos.ao_flight_drogue]));
			System.out.printf("Main rate:   %6.0f m/s  %6.0f ft/s\n",
					  stats.state_baro_speed[Altos.ao_flight_main],
					  AltosConvert.meters_to_feet(stats.state_baro_speed[Altos.ao_flight_main]));
			System.out.printf("Flight time: %6.0f s\n",
					  stats.state_end[Altos.ao_flight_main] -
					  stats.state_start[Altos.ao_flight_boost]);
		} catch (InterruptedException ie) {
		} catch (IOException ie) {
		}
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
	
	public static void main(final String[] args) {
		load_library(null);
		try {
			UIManager.setLookAndFeel(AltosUIPreferences.look_and_feel());
		} catch (Exception e) {
		}
		/* Handle batch-mode */
		if (args.length == 0) {
			AltosUI altosui = new AltosUI();
			altosui.setVisible(true);

			java.util.List<AltosDevice> devices = AltosUSBDevice.list(Altos.product_basestation);
			for (AltosDevice device : devices)
				altosui.telemetry_window(device);
		} else {
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
						AltosSiteMap.prefetchMaps(lat, lon, 5, 5);
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
				else if (args[i].startsWith("--"))
					help(1);
				else {
					switch (process) {
					case process_none:
					case process_graph:
						process_graph(args[i]);
						break;
					case process_replay:
						process_replay(args[i]);
						break;
					case process_kml:
						process_kml(args[i]);
						break;
					case process_csv:
						process_csv(args[i]);
						break;
					case process_summary:
						process_summary(args[i]);
						break;
					}
				}
			}
		}
	}
}

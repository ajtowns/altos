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

public class AltosUI extends JFrame {
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
			AltosFlightReader reader = new AltosTelemetryReader(device);
			if (reader != null)
				new AltosFlightUI(voice, reader, device.getSerial());
		} catch (FileNotFoundException ee) {
			JOptionPane.showMessageDialog(AltosUI.this,
						      String.format("Cannot open device \"%s\"",
								    device.toShortString()),
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

		java.net.URL imgURL = AltosUI.class.getResource("/altus-metrum-16x16.jpg");
		if (imgURL != null)
			setIconImage(new ImageIcon(imgURL).getImage());

		AltosPreferences.set_component(this);

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
		b = addButton(1, 0, "Save Flight Data");
		b.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						SaveFlightData();
					}
				});
		b = addButton(2, 0, "Replay Flight");
		b.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						Replay();
					}
				});
		b = addButton(3, 0, "Graph Data");
		b.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						GraphData();
					}
				});
		b = addButton(4, 0, "Export Data");
		b.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						ExportData();
					}
				});
		b = addButton(0, 1, "Configure Altimeter");
		b.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						ConfigureTeleMetrum();
					}
				});

		b = addButton(1, 1, "Configure AltosUI");
		b.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					ConfigureAltosUI();
				}
			});

		b = addButton(2, 1, "Flash Image");
		b.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					FlashImage();
				}
			});

		b = addButton(3, 1, "Fire Igniter");
		b.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					FireIgniter();
				}
			});

		b = addButton(4, 1, "Quit");
		b.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					System.exit(0);
				}
			});


		b = addButton(0, 2, "Scan Channels");
		b.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					ScanChannels();
				}
			});

		b = addButton(1, 2, "Load Maps");
		b.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					LoadMaps();
				}
			});

		b = addButton(2, 2, "Monitor Idle");
		b.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					IdleMonitor();
				}
			});

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
						     AltosPreferences.callsign());
		if (result != null)
			AltosPreferences.set_callsign(result);
	}

	void ConfigureTeleMetrum() {
		new AltosConfig(AltosUI.this);
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
			else
				return new AltosTelemetryIterable(in);
		} catch (FileNotFoundException fe) {
			System.out.printf("Cannot open '%s'\n", filename);
			return null;
		}
	}

	static AltosWriter open_csv(String filename) {
		File file = new File (filename);
		try {
			return new AltosCSV(file);
		} catch (FileNotFoundException fe) {
			System.out.printf("Cannot open '%s'\n", filename);
			return null;
		}
	}

	static AltosWriter open_kml(String filename) {
		File file = new File (filename);
		try {
			return new AltosKML(file);
		} catch (FileNotFoundException fe) {
			System.out.printf("Cannot open '%s'\n", filename);
			return null;
		}
	}

	static final int process_none = 0;
	static final int process_csv = 1;
	static final int process_kml = 2;
	static final int process_graph = 3;
	static final int process_replay = 4;

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

	static void process_replay(String filename) {
		FileInputStream in;
		try {
			in = new FileInputStream(filename);
		} catch (Exception e) {
			System.out.printf("Failed to open file '%s'\n", filename);
			return;
		}
		AltosRecordIterable recs;
		AltosReplayReader reader;
		if (filename.endsWith("eeprom")) {
			recs = new AltosEepromIterable(in);
		} else {
			recs = new AltosTelemetryIterable(in);
		}
		reader = new AltosReplayReader(recs.iterator(), new File(filename));
		AltosFlightUI flight_ui = new AltosFlightUI(new AltosVoice(), reader);
		flight_ui.set_exit_on_close();
	}

	static void process_graph(String filename) {
		FileInputStream in;
		try {
			in = new FileInputStream(filename);
		} catch (Exception e) {
			System.out.printf("Failed to open file '%s'\n", filename);
			return;
		}
		AltosRecordIterable recs;
		if (filename.endsWith("eeprom")) {
			recs = new AltosEepromIterable(in);
		} else {
			recs = new AltosTelemetryIterable(in);
		}
		try {
			new AltosGraphUI(recs);
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
					}
				}
			}
		}
	}
}

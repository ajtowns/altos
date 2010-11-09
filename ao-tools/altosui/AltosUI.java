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
import altosui.AltosFlightInfoTableModel;
import altosui.AltosFlashUI;
import altosui.AltosLogfileChooser;
import altosui.AltosCSVUI;
import altosui.AltosLine;
import altosui.AltosStatusTable;
import altosui.AltosInfoTable;
import altosui.AltosDisplayThread;

import libaltosJNI.*;

public class AltosUI extends JFrame {
	public AltosVoice voice = new AltosVoice();

	public static boolean load_library(Frame frame) {
		if (!AltosDevice.load_library()) {
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
								    device.getPath()),
						      "Cannot open target device",
						      JOptionPane.ERROR_MESSAGE);
		} catch (IOException ee) {
			JOptionPane.showMessageDialog(AltosUI.this,
						      device.getPath(),
						      "Unkonwn I/O error",
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

		AltosPreferences.init(this);

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
		b = addButton(0, 1, "Graph Data");
		b.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						GraphData();
					}
				});
		b = addButton(1, 1, "Export Data");
		b.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						ExportData();
					}
				});
		b = addButton(2, 1, "Configure TeleMetrum");
		b.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						ConfigureTeleMetrum();
					}
				});

		setTitle("AltOS");

		createMenu();

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
								AltosDevice.product_basestation);

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
		new AltosFlashUI(AltosUI.this);
	}

	/*
	 * Replay a flight from telemetry data
	 */
	private void Replay() {
		AltosLogfileChooser chooser = new AltosLogfileChooser(
			AltosUI.this);
		AltosRecordIterable iterable = chooser.runDialog();
		if (iterable != null) {
			AltosFlightReader reader = new AltosReplayReader(iterable.iterator(),
									 chooser.filename());
			new AltosFlightUI(voice, reader);
		}
	}

	/* Connect to TeleMetrum, either directly or through
	 * a TeleDongle over the packet link
	 */
	private void SaveFlightData() {
		new AltosEepromDownload(AltosUI.this);
	}

	/* Load a flight log file and write out a CSV file containing
	 * all of the data in standard units
	 */

	private void ExportData() {
		new AltosCSVUI(AltosUI.this);
	}

	/* Load a flight log CSV file and display a pretty graph.
	 */

	private void GraphData() {
		new AltosGraphUI(AltosUI.this);
	}

	/* Create the AltosUI menus
	 */
	private void createMenu() {
		JMenuBar menubar = new JMenuBar();
		JMenu menu;
		JMenuItem item;
		JRadioButtonMenuItem radioitem;

		// File menu
		{
			menu = new JMenu("File");
			menu.setMnemonic(KeyEvent.VK_F);
			menubar.add(menu);

			item = new JMenuItem("Flash Image",KeyEvent.VK_I);
			item.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						FlashImage();
					}
				});
			menu.add(item);

			item = new JMenuItem("Export Data",KeyEvent.VK_E);
			item.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						ExportData();
					}
				});
			menu.add(item);

			item = new JMenuItem("Graph Data",KeyEvent.VK_G);
			item.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						GraphData();
					}
				});
			menu.add(item);

			item = new JMenuItem("Quit",KeyEvent.VK_Q);
			item.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_Q,
								   ActionEvent.CTRL_MASK));
			item.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						System.out.printf("exiting\n");
						System.exit(0);
					}
				});
			menu.add(item);
		}

		// Device menu
		if (false) {
			menu = new JMenu("Device");
			menu.setMnemonic(KeyEvent.VK_D);
			menubar.add(menu);

			item = new JMenuItem("Connect to Device",KeyEvent.VK_C);
			item.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						ConnectToDevice();
					}
				});
			menu.add(item);

			menu.addSeparator();

			item = new JMenuItem("Set Callsign",KeyEvent.VK_S);
			item.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						ConfigureCallsign();
					}
				});

			menu.add(item);

			item = new JMenuItem("Configure TeleMetrum device",KeyEvent.VK_T);
			item.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						ConfigureTeleMetrum();
					}
				});

			menu.add(item);
		}
		// Log menu
		{
			menu = new JMenu("Log");
			menu.setMnemonic(KeyEvent.VK_L);
			menubar.add(menu);

			item = new JMenuItem("New Log",KeyEvent.VK_N);
			item.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
					}
				});
			menu.add(item);

			item = new JMenuItem("Configure Log",KeyEvent.VK_C);
			item.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						AltosPreferences.ConfigureLog();
					}
				});
			menu.add(item);
		}
		// Voice menu
		{
			menu = new JMenu("Voice", true);
			menu.setMnemonic(KeyEvent.VK_V);
			menubar.add(menu);

			radioitem = new JRadioButtonMenuItem("Enable Voice", AltosPreferences.voice());
			radioitem.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						JRadioButtonMenuItem item = (JRadioButtonMenuItem) e.getSource();
						boolean enabled = item.isSelected();
						AltosPreferences.set_voice(enabled);
						if (enabled)
							voice.speak_always("Enable voice.");
						else
							voice.speak_always("Disable voice.");
					}
				});
			menu.add(radioitem);
			item = new JMenuItem("Test Voice",KeyEvent.VK_T);
			item.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						voice.speak("That's one small step for man; one giant leap for mankind.");
					}
				});
			menu.add(item);
		}
		this.setJMenuBar(menubar);
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

	static final int process_csv = 1;
	static final int process_kml = 2;

	static void process_file(String input, int process) {
		AltosRecordIterable iterable = open_logfile(input);
		if (iterable == null)
			return;
		if (process == 0)
			process = process_csv;
		if ((process & process_csv) != 0) {
			String output = Altos.replace_extension(input,".csv");
			System.out.printf("Processing \"%s\" to \"%s\"\n", input, output);
			if (input.equals(output)) {
				System.out.printf("Not processing '%s'\n", input);
			} else {
				AltosWriter writer = open_csv(output);
				if (writer != null) {
					writer.write(iterable);
					writer.close();
				}
			}
		}
		if ((process & process_kml) != 0) {
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
	}

	public static void main(final String[] args) {
		int	process = 0;
		/* Handle batch-mode */
		if (args.length > 0) {
			for (int i = 0; i < args.length; i++) {
				if (args[i].equals("--kml"))
					process |= process_kml;
				else if (args[i].equals("--csv"))
					process |= process_csv;
				else
					process_file(args[i], process);
			}
		} else {
			AltosUI altosui = new AltosUI();
			altosui.setVisible(true);

			AltosDevice[] devices = AltosDevice.list(AltosDevice.product_basestation);
			for (int i = 0; i < devices.length; i++)
				altosui.telemetry_window(devices[i]);
		}
	}
}

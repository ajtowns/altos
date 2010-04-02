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
import java.io.*;
import java.util.*;
import java.text.*;
import gnu.io.CommPortIdentifier;

import altosui.AltosSerial;
import altosui.AltosSerialMonitor;

class AltosUIMonitor implements AltosSerialMonitor {
	public void data(String data) {
		System.out.println(data);
	}
}

public class AltosUI extends JFrame {
	private int channel = -1;

	private JTable flightStatus;
	private JTable flightInfo;
	private AltosSerial serialLine;

	public AltosUI() {

		String[] statusNames = { "Height (m)", "State", "RSSI (dBm)", "Speed (m/s)" };
		Object[][] statusData = { { "0", "pad", "-50", "0" } };

		flightStatus = new JTable(statusData, statusNames);

		flightStatus.setShowGrid(false);

		this.add(flightStatus);

		setTitle("AltOS");

		createMenu();

		serialLine = new AltosSerial();
		serialLine.monitor(new AltosUIMonitor());
		int dpi = Toolkit.getDefaultToolkit().getScreenResolution();
		this.setSize(new Dimension (dpi * 5, dpi * 4));
		this.validate();
		setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
		addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosing(WindowEvent e) {
				System.exit(0);
			}
		});
	}

	final JFileChooser deviceChooser = new JFileChooser();

	private void PickSerialDevice() {
		java.util.Enumeration<CommPortIdentifier> port_list = CommPortIdentifier.getPortIdentifiers();
		while (port_list.hasMoreElements()) {
			CommPortIdentifier identifier = port_list.nextElement();
			System.out.println("Serial port " + identifier.getName());
		}
	}

	private void ConnectToDevice() {
		PickSerialDevice();
		int returnVal = deviceChooser.showOpenDialog(AltosUI.this);

		if (returnVal == JFileChooser.APPROVE_OPTION) {
			File file = deviceChooser.getSelectedFile();
			try {
				serialLine.open(file);
			} catch (FileNotFoundException ee) {
				JOptionPane.showMessageDialog(AltosUI.this,
							      file.getName(),
							      "Cannot open serial port",
							      JOptionPane.ERROR_MESSAGE);
			}
		}
	}

	String readline(FileInputStream s) throws IOException {
		int c;
		String	line = "";

		while ((c = s.read()) != -1) {
			if (c == '\r')
				continue;
			if (c == '\n')
				return line;
			line = line + (char) c;
		}
		return null;
	}

	private void Replay() {
//		int returnVal = deviceChooser.showOpenDialog(AltosUI.this);

		/*		if (returnVal == JFileChooser.APPROVE_OPTION) */ {
//			File file = deviceChooser.getSelectedFile();
//			String	filename = file.getName();
			String	filename = "/home/keithp/src/cc1111/flights/2010-02-13-serial-051-flight-002.telem";
			try {
//				FileInputStream	replay = new FileInputStream(file);
				FileInputStream	replay = new FileInputStream(filename);
				String	line;

				try {
					while ((line = readline(replay)) != null) {
						try {
							AltosTelemetry	t = new AltosTelemetry(line);
							System.out.println ("Version " + t.version + t.callsign);
						} catch (ParseException pp) {
							JOptionPane.showMessageDialog(AltosUI.this,
										      line,
										      "error parsing",
										      JOptionPane.ERROR_MESSAGE);
							break;
						}
					}
				} catch (IOException ee) {
					JOptionPane.showMessageDialog(AltosUI.this,
								      filename,
								      "error reading",
								      JOptionPane.ERROR_MESSAGE);
				} finally {
					try {
						replay.close();
					} catch (IOException e) {}
				}
			} catch (FileNotFoundException ee) {
				JOptionPane.showMessageDialog(AltosUI.this,
							      filename,
							      "Cannot open serial port",
							      JOptionPane.ERROR_MESSAGE);
			}
		}
	}

	private void SaveFlightData() {
	}

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

			item = new JMenuItem("Quit",KeyEvent.VK_Q);
			item.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_Q,
								   ActionEvent.CTRL_MASK));
			item.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						System.exit(0);
					}
				});
			menu.add(item);
		}

		// Device menu
		{
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

			item = new JMenuItem("Disconnect from Device",KeyEvent.VK_D);
			item.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						serialLine.close();
					}
				});
			menu.add(item);

			menu.addSeparator();

			item = new JMenuItem("Save Flight Data",KeyEvent.VK_S);
			item.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						SaveFlightData();
					}
				});
			menu.add(item);

			item = new JMenuItem("Replay",KeyEvent.VK_R);
			item.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						Replay();
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
					}
				});
			menu.add(item);
		}
		// Voice menu
		{
			menu = new JMenu("Voice", true);
			menu.setMnemonic(KeyEvent.VK_V);
			menubar.add(menu);

			radioitem = new JRadioButtonMenuItem("Enable Voice");
			radioitem.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
					}
				});
			menu.add(radioitem);
		}

		// Channel menu
		{
			menu = new JMenu("Channel", true);
			menu.setMnemonic(KeyEvent.VK_C);
			menubar.add(menu);
			ButtonGroup group = new ButtonGroup();

			for (int c = 0; c <= 9; c++) {
				radioitem = new JRadioButtonMenuItem(String.format("Channel %1d (%7.3fMHz)", c,
										   434.550 + c * 0.1),
								     c == 0);
				radioitem.setActionCommand(String.format("%d", c));
				radioitem.addActionListener(new ActionListener() {
						public void actionPerformed(ActionEvent e) {
							System.out.println("Command: " + e.getActionCommand() + " param: " +
									   e.paramString());
						}
					});
				menu.add(radioitem);
				group.add(radioitem);
			}
		}

		this.setJMenuBar(menubar);

	}
	public static void main(final String[] args) {
		AltosUI altosui = new AltosUI();
		altosui.setVisible(true);
	}
}
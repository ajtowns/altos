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
import javax.swing.table.AbstractTableModel;
import java.io.*;
import java.util.*;
import java.text.*;
import gnu.io.CommPortIdentifier;

import altosui.AltosSerial;
import altosui.AltosSerialMonitor;
import altosui.AltosTelemetry;
import altosui.AltosState;

class AltosUIMonitor implements AltosSerialMonitor {
	public void data(String data) {
		System.out.println(data);
	}
}

class AltosFlightStatusTableModel extends AbstractTableModel {
	private String[] columnNames = {"Height (m)", "State", "RSSI (dBm)", "Speed (m/s)" };
	private Object[] data = { 0, "idle", 0, 0 };

	public int getColumnCount() { return columnNames.length; }
	public int getRowCount() { return 1; }
	public String getColumnName(int col) { return columnNames[col]; }
	public Object getValueAt(int row, int col) { return data[col]; }

	public void setValueAt(Object value, int col) {
		data[col] = value;
		fireTableCellUpdated(0, col);
	}

	public void setValueAt(Object value, int row, int col) {
		setValueAt(value, col);
	}

	public void set(AltosState state) {
		setValueAt(String.format("%1.0f", state.height), 0);
		setValueAt(state.data.state, 1);
		setValueAt(state.data.rssi, 2);
		double speed = state.baro_speed;
		if (state.ascent)
			speed = state.speed;
		setValueAt(String.format("%1.0f", speed), 3);
	}
}

class AltosFlightInfoTableModel extends AbstractTableModel {
	private String[] columnNames = {"Field", "Value"};

	class InfoLine {
		String	name;
		String	value;

		public InfoLine(String n, String v) {
			name = n;
			value = v;
		}
	}

	private ArrayList<InfoLine> rows = new ArrayList<InfoLine>();

	public int getColumnCount() { return columnNames.length; }
	public String getColumnName(int col) { return columnNames[col]; }

	public int getRowCount() { return 20; }

	public Object getValueAt(int row, int col) {
		if (row >= rows.size())
			return "";
		if (col == 0)
			return rows.get(row).name;
		else
			return rows.get(row).value;
	}

	int	current_row = 0;
	int	prev_num_rows = 0;

	public void resetRow() {
		current_row = 0;
	}
	public void addRow(String name, String value) {
		if (current_row >= rows.size())
			rows.add(current_row, new InfoLine(name, value));
		else
			rows.set(current_row, new InfoLine(name, value));
		current_row++;
	}
	public void finish() {
		if (current_row > prev_num_rows) {
			fireTableRowsInserted(prev_num_rows, current_row - 1);
			prev_num_rows = current_row;
		}
		fireTableDataChanged();
	}
}

public class AltosUI extends JFrame {
	private int channel = -1;

	private AltosFlightStatusTableModel flightStatusModel;
	private JTable flightStatus;

	static final int info_columns = 3;

	private AltosFlightInfoTableModel[] flightInfoModel;
	private JTable[] flightInfo;
	private AltosSerial serialLine;
	private Box[] ibox;
	private Box vbox;
	private Box hbox;

	public AltosUI() {

		String[] statusNames = { "Height (m)", "State", "RSSI (dBm)", "Speed (m/s)" };
		Object[][] statusData = { { "0", "pad", "-50", "0" } };

		vbox = Box.createVerticalBox();
		this.add(vbox);

		flightStatusModel = new AltosFlightStatusTableModel();
		flightStatus = new JTable(flightStatusModel);

		flightStatus.setShowGrid(false);

		flightInfo = new JTable[3];
		flightInfoModel = new AltosFlightInfoTableModel[3];
		ibox = new Box[3];

		vbox.add(flightStatus.getTableHeader());
		vbox.add(flightStatus);

		hbox = Box.createHorizontalBox();
		vbox.add(hbox);

		for (int i = 0; i < info_columns; i++) {
			ibox[i] = Box.createVerticalBox();
			flightInfoModel[i] = new AltosFlightInfoTableModel();
			flightInfo[i] = new JTable(flightInfoModel[i]);
			flightInfo[i].setShowGrid(true);
			ibox[i].add(flightInfo[i].getTableHeader());
			ibox[i].add(flightInfo[i]);
			hbox.add(ibox[i]);
		}

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

	public void info_reset() {
		for (int i = 0; i < info_columns; i++)
			flightInfoModel[i].resetRow();
	}

	public void info_add_row(int col, String name, String value) {
		flightInfoModel[col].addRow(name, value);
	}

	public void info_add_row(int col, String name, String format, Object value) {
		flightInfoModel[col].addRow(name, String.format(format, value));
	}

	public void info_add_row(int col, String name, String format, Object v1, Object v2) {
		flightInfoModel[col].addRow(name, String.format(format, v1, v2));
	}

	public void info_add_row(int col, String name, String format, Object v1, Object v2, Object v3) {
		flightInfoModel[col].addRow(name, String.format(format, v1, v2, v3));
	}

	public void info_add_deg(int col, String name, double v, int pos, int neg) {
		int	c = pos;
		if (v < 0) {
			c = neg;
			v = -v;
		}
		double	deg = Math.floor(v);
		double	min = (v - deg) * 60;

		flightInfoModel[col].addRow(name, String.format("%3.0f°%08.5f'", deg, min));
	}

	public void info_finish() {
		for (int i = 0; i < info_columns; i++)
			flightInfoModel[i].finish();
	}

	static final int MIN_PAD_SAMPLES = 10;

	public void show(AltosState state) {
		flightStatusModel.set(state);

		info_reset();
		if (state.npad >= MIN_PAD_SAMPLES)
			info_add_row(0, "Ground state", "%s", "ready");
		else
			info_add_row(0, "Ground state", "waiting for gps (%d)",
				     MIN_PAD_SAMPLES - state.npad);
		info_add_row(0, "Rocket state", "%s", state.data.state);
		info_add_row(0, "Callsign", "%s", state.data.callsign);
		info_add_row(0, "Rocket serial", "%d", state.data.serial);
		info_add_row(0, "Rocket flight", "%d", state.data.flight);

		info_add_row(0, "RSSI", "%6ddBm", state.data.rssi);
		info_add_row(0, "Height", "%6.0fm", state.height);
		info_add_row(0, "Max height", "%6.0fm", state.max_height);
		info_add_row(0, "Acceleration", "%7.1fm/s²", state.acceleration);
		info_add_row(0, "Max acceleration", "%7.1fm/s²", state.max_acceleration);
		info_add_row(0, "Speed", "%7.1fm/s", state.ascent ? state.speed : state.baro_speed);
		info_add_row(0, "Max Speed", "%7.1fm/s", state.max_speed);
		info_add_row(0, "Temperature", "%6.2f°C", state.temperature);
		info_add_row(0, "Battery", "%5.2fV", state.battery);
		info_add_row(0, "Drogue", "%5.2fV", state.drogue_sense);
		info_add_row(0, "Main", "%5.2fV", state.main_sense);
		info_add_row(0, "Pad altitude", "%6.0fm", state.ground_altitude);
		if (state.gps != null)
			info_add_row(1, "Satellites", "%d", state.gps.nsat);
		else
			info_add_row(1, "Satellites", "%d", 0);
		if (state.gps != null && state.gps.gps_locked) {
			info_add_row(1, "GPS", "locked");
		} else if (state.gps != null && state.gps.gps_connected) {
			info_add_row(1, "GPS", "unlocked");
		} else {
			info_add_row(1, "GPS", "not available");
		}
		if (state.gps != null) {
			info_add_deg(1, "Latitude", state.gps.lat, 'N', 'S');
			info_add_deg(1, "Longitude", state.gps.lon, 'E', 'W');
			info_add_row(1, "GPS altitude", "%d", state.gps.alt);
			info_add_row(1, "GPS height", "%d", state.gps_height);
			info_add_row(1, "GPS date", "%04d-%02d-%02d",
				       state.gps.gps_time.year,
				       state.gps.gps_time.month,
				       state.gps.gps_time.day);
			info_add_row(1, "GPS time", "%02d:%02d:%02d",
				       state.gps.gps_time.hour,
				       state.gps.gps_time.minute,
				       state.gps.gps_time.second);
			info_add_row(1, "GPS ground speed", "%7.1fm/s %d°",
				       state.gps.ground_speed,
				       state.gps.course);
			info_add_row(1, "GPS climb rate", "%7.1fm/s",
				     state.gps.climb_rate);
			info_add_row(1, "GPS precision", "%4.1f(hdop) %3dm(h) %3dm(v)",
				     state.gps.hdop, state.gps.h_error, state.gps.v_error);
		}
		if (state.npad > 0) {
			info_add_row(1, "Distance from pad", "%5.0fm", state.from_pad.distance);
			info_add_row(1, "Direction from pad", "%4.0f°", state.from_pad.bearing);
			info_add_deg(1, "Pad latitude", state.pad_lat, 'N', 'S');
			info_add_deg(1, "Pad longitude", state.pad_lon, 'E', 'W');
			info_add_row(1, "Pad GPS alt", "%gm", state.pad_alt);
		}
		if (state.gps != null && state.gps.gps_connected) {
			int	nsat_vis = 0;
			int	c;

			if (state.gps.cc_gps_sat == null)
				info_add_row(2, "Satellites Visible", "%d", 0);
			else {
				info_add_row(2, "Satellites Visible", "%d", state.gps.cc_gps_sat.length);
				for (c = 0; c < state.gps.cc_gps_sat.length; c++) {
					info_add_row(2, "Satellite id,C/N0",
						     "%3d,%2d",
						     state.gps.cc_gps_sat[c].svid,
						     state.gps.cc_gps_sat[c].c_n0);
				}
			}
		}
		info_finish();
	}


	final JFileChooser deviceChooser = new JFileChooser();
	final JFileChooser logdirChooser = new JFileChooser();
	final String logdirName = "TeleMetrum";
	File logdir = null;

	private void setLogdir() {
		if (logdir == null)
			logdir = new File(logdirChooser.getCurrentDirectory(), logdirName);
		logdirChooser.setCurrentDirectory(logdir);
	}

	private void makeLogdir() {
		setLogdir();
		if (!logdir.exists()) {
			if (!logdir.mkdirs())
				JOptionPane.showMessageDialog(AltosUI.this,
							      logdir.getName(),
							      "Cannot create directory",
							      JOptionPane.ERROR_MESSAGE);
		} else if (!logdir.isDirectory()) {
			JOptionPane.showMessageDialog(AltosUI.this,
						      logdir.getName(),
						      "Is not a directory",
						      JOptionPane.ERROR_MESSAGE);
		}
	}

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

	/*
	 * Open an existing telemetry file and replay it in realtime
	 */

	class ReplayThread extends Thread {
		FileInputStream	replay;
		String filename;

		ReplayThread(FileInputStream in, String name) {
			replay = in;
			filename = name;
		}

		public void run() {
			String	line;
			AltosState	state = null;
			try {
				while ((line = readline(replay)) != null) {
					try {
						AltosTelemetry	t = new AltosTelemetry(line);
						state = new AltosState(t, state);
						show(state);
						try {
							if (state.state > AltosTelemetry.ao_flight_pad)
								Thread.sleep((int) (state.time_change * 1000));
						} catch (InterruptedException e) {}
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
		}
	}

	private void Replay() {
		setLogdir();
		logdirChooser.setDialogTitle("Select Telemetry File");
		logdirChooser.setFileFilter(new FileNameExtensionFilter("Telemetry file", "telem"));
		int returnVal = logdirChooser.showOpenDialog(AltosUI.this);

		if (returnVal == JFileChooser.APPROVE_OPTION) {
			File file = logdirChooser.getSelectedFile();
			if (file == null)
				System.out.println("No file selected?");
			String	filename = file.getName();
			try {
				FileInputStream	replay = new FileInputStream(file);
				ReplayThread	thread = new ReplayThread(replay, filename);
				thread.start();
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
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

import altosui.AltosSerial;
import altosui.AltosSerialMonitor;
import altosui.AltosTelemetry;
import altosui.AltosState;
import altosui.AltosDeviceDialog;
import altosui.AltosPreferences;
import altosui.AltosLog;
import altosui.AltosVoice;

import libaltosJNI.*;

class AltosFlightStatusTableModel extends AbstractTableModel {
	private String[] columnNames = {"Height (m)", "State", "RSSI (dBm)", "Speed (m/s)" };
	private Object[] data = { 0, "idle", 0, 0 };

	public int getColumnCount() { return columnNames.length; }
	public int getRowCount() { return 2; }
	public Object getValueAt(int row, int col) {
		if (row == 0)
			return columnNames[col];
		return data[col];
	}

	public void setValueAt(Object value, int col) {
		data[col] = value;
		fireTableCellUpdated(1, col);
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
	private AltosSerial serial_line;
	private AltosLog altos_log;
	private Box[] ibox;
	private Box vbox;
	private Box hbox;

	private Font statusFont = new Font("SansSerif", Font.BOLD, 24);
	private Font infoLabelFont = new Font("SansSerif", Font.PLAIN, 14);
	private Font infoValueFont = new Font("Monospaced", Font.PLAIN, 14);

	public AltosVoice voice = new AltosVoice();

	public AltosUI() {

		String[] statusNames = { "Height (m)", "State", "RSSI (dBm)", "Speed (m/s)" };
		Object[][] statusData = { { "0", "pad", "-50", "0" } };

		AltosPreferences.init(this);

		vbox = Box.createVerticalBox();
		this.add(vbox);

		flightStatusModel = new AltosFlightStatusTableModel();
		flightStatus = new JTable(flightStatusModel);
		flightStatus.setFont(statusFont);
		TableColumnModel tcm = flightStatus.getColumnModel();
		for (int i = 0; i < flightStatusModel.getColumnCount(); i++) {
			DefaultTableCellRenderer       r = new DefaultTableCellRenderer();
			r.setFont(statusFont);
			r.setHorizontalAlignment(SwingConstants.CENTER);
			tcm.getColumn(i).setCellRenderer(r);
		}

		FontMetrics	statusMetrics = flightStatus.getFontMetrics(statusFont);
		int statusHeight = (statusMetrics.getHeight() + statusMetrics.getLeading()) * 15 / 10;
		flightStatus.setRowHeight(statusHeight);
		flightStatus.setShowGrid(false);

		vbox.add(flightStatus);

		hbox = Box.createHorizontalBox();
		vbox.add(hbox);

		flightInfo = new JTable[3];
		flightInfoModel = new AltosFlightInfoTableModel[3];
		ibox = new Box[3];
		FontMetrics	infoValueMetrics = flightStatus.getFontMetrics(infoValueFont);
		int infoHeight = (infoValueMetrics.getHeight() + infoValueMetrics.getLeading()) * 20 / 10;

		for (int i = 0; i < info_columns; i++) {
			ibox[i] = Box.createVerticalBox();
			flightInfoModel[i] = new AltosFlightInfoTableModel();
			flightInfo[i] = new JTable(flightInfoModel[i]);
			flightInfo[i].setFont(infoValueFont);
			flightInfo[i].setRowHeight(infoHeight);
			flightInfo[i].setShowGrid(true);
			ibox[i].add(flightInfo[i].getTableHeader());
			ibox[i].add(flightInfo[i]);
			hbox.add(ibox[i]);
		}

		setTitle("AltOS");

		createMenu();

		serial_line = new AltosSerial();
		altos_log = new AltosLog(serial_line);
		int dpi = Toolkit.getDefaultToolkit().getScreenResolution();
		this.setSize(new Dimension (infoValueMetrics.charWidth('0') * 6 * 20,
					    statusHeight * 4 + infoHeight * 17));
		this.validate();
		setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
		addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosing(WindowEvent e) {
				System.exit(0);
			}
		});
		voice.speak("Rocket flight monitor ready.");
	}

	public void info_reset() {
		for (int i = 0; i < info_columns; i++)
			flightInfoModel[i].resetRow();
	}

	public void info_add_row(int col, String name, String value) {
		flightInfoModel[col].addRow(name, value);
	}

	public void info_add_row(int col, String name, String format, Object... parameters) {
		flightInfoModel[col].addRow(name, String.format(format, parameters));
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

	public void show(AltosState state) {
		flightStatusModel.set(state);

		info_reset();
		if (state.gps_ready)
			info_add_row(0, "Ground state", "%s", "ready");
		else
			info_add_row(0, "Ground state", "wait (%d)",
				     state.gps_waiting);
		info_add_row(0, "Rocket state", "%s", state.data.state);
		info_add_row(0, "Callsign", "%s", state.data.callsign);
		info_add_row(0, "Rocket serial", "%6d", state.data.serial);
		info_add_row(0, "Rocket flight", "%6d", state.data.flight);

		info_add_row(0, "RSSI", "%6d    dBm", state.data.rssi);
		info_add_row(0, "Height", "%6.0f    m", state.height);
		info_add_row(0, "Max height", "%6.0f    m", state.max_height);
		info_add_row(0, "Acceleration", "%8.1f  m/s²", state.acceleration);
		info_add_row(0, "Max acceleration", "%8.1f  m/s²", state.max_acceleration);
		info_add_row(0, "Speed", "%8.1f  m/s", state.ascent ? state.speed : state.baro_speed);
		info_add_row(0, "Max Speed", "%8.1f  m/s", state.max_speed);
		info_add_row(0, "Temperature", "%9.2f °C", state.temperature);
		info_add_row(0, "Battery", "%9.2f V", state.battery);
		info_add_row(0, "Drogue", "%9.2f V", state.drogue_sense);
		info_add_row(0, "Main", "%9.2f V", state.main_sense);
		info_add_row(0, "Pad altitude", "%6.0f    m", state.ground_altitude);
		if (state.gps == null) {
			info_add_row(1, "GPS", "not available");
		} else {
			if (state.data.gps.gps_locked)
				info_add_row(1, "GPS", "   locked");
			else if (state.data.gps.gps_connected)
				info_add_row(1, "GPS", " unlocked");
			else
				info_add_row(1, "GPS", "  missing");
			info_add_row(1, "Satellites", "%6d", state.data.gps.nsat);
			info_add_deg(1, "Latitude", state.gps.lat, 'N', 'S');
			info_add_deg(1, "Longitude", state.gps.lon, 'E', 'W');
			info_add_row(1, "GPS altitude", "%6d", state.gps.alt);
			info_add_row(1, "GPS height", "%6.0f", state.gps_height);

			/* The SkyTraq GPS doesn't report these values */
			if (false) {
				info_add_row(1, "GPS ground speed", "%8.1f m/s %3d°",
					     state.gps.ground_speed,
					     state.gps.course);
				info_add_row(1, "GPS climb rate", "%8.1f m/s",
					     state.gps.climb_rate);
				info_add_row(1, "GPS error", "%6d m(h)%3d m(v)",
					     state.gps.h_error, state.gps.v_error);
			}
			info_add_row(1, "GPS hdop", "%8.1f", state.gps.hdop);

			if (state.npad > 0) {
				if (state.from_pad != null) {
					info_add_row(1, "Distance from pad", "%6.0f m", state.from_pad.distance);
					info_add_row(1, "Direction from pad", "%6.0f°", state.from_pad.bearing);
				} else {
					info_add_row(1, "Distance from pad", "unknown");
					info_add_row(1, "Direction from pad", "unknown");
				}
				info_add_deg(1, "Pad latitude", state.pad_lat, 'N', 'S');
				info_add_deg(1, "Pad longitude", state.pad_lon, 'E', 'W');
				info_add_row(1, "Pad GPS alt", "%6.0f m", state.pad_alt);
			}
			info_add_row(1, "GPS date", "%04d-%02d-%02d",
				       state.gps.gps_time.year,
				       state.gps.gps_time.month,
				       state.gps.gps_time.day);
			info_add_row(1, "GPS time", "  %02d:%02d:%02d",
				       state.gps.gps_time.hour,
				       state.gps.gps_time.minute,
				       state.gps.gps_time.second);
			int	nsat_vis = 0;
			int	c;

			if (state.gps.cc_gps_sat == null)
				info_add_row(2, "Satellites Visible", "%4d", 0);
			else {
				info_add_row(2, "Satellites Visible", "%4d", state.gps.cc_gps_sat.length);
				for (c = 0; c < state.gps.cc_gps_sat.length; c++) {
					info_add_row(2, "Satellite id,C/N0",
						     "%4d, %4d",
						     state.gps.cc_gps_sat[c].svid,
						     state.gps.cc_gps_sat[c].c_n0);
				}
			}
		}
		info_finish();
	}

	class IdleThread extends Thread {

		private AltosState state;
		int	reported_landing;

		public void report(boolean last) {
			if (state == null)
				return;

			/* reset the landing count once we hear about a new flight */
			if (state.state < AltosTelemetry.ao_flight_drogue)
				reported_landing = 0;

			/* Shut up once the rocket is on the ground */
			if (reported_landing > 2) {
				return;
			}

			/* If the rocket isn't on the pad, then report height */
			if (state.state > AltosTelemetry.ao_flight_pad) {
				voice.speak("%d meters", (int) (state.height + 0.5));
			} else {
				reported_landing = 0;
			}

			/* If the rocket is coming down, check to see if it has landed;
			 * either we've got a landed report or we haven't heard from it in
			 * a long time
			 */
			if (!state.ascent &&
			    (last ||
			     System.currentTimeMillis() - state.report_time >= 15000 ||
			     state.state == AltosTelemetry.ao_flight_landed))
			{
				if (Math.abs(state.baro_speed) < 20 && state.height < 100)
					voice.speak("rocket landed safely");
				else
					voice.speak("rocket may have crashed");
				if (state.from_pad != null)
					voice.speak("bearing %d degrees, range %d meters",
						    (int) (state.from_pad.bearing + 0.5),
						    (int) (state.from_pad.distance + 0.5));
				++reported_landing;
			}
		}

		public void run () {

			reported_landing = 0;
			state = null;
			try {
				for (;;) {
					Thread.sleep(10000);
					report(false);
				}
			} catch (InterruptedException ie) {
			}
		}

		public void notice(AltosState new_state) {
			state = new_state;
		}
	}

	private void tell(AltosState state, AltosState old_state) {
		if (old_state == null || old_state.state != state.state) {
			voice.speak(state.data.state);
			if ((old_state == null || old_state.state <= AltosTelemetry.ao_flight_boost) &&
			    state.state > AltosTelemetry.ao_flight_boost) {
				voice.speak("max speed: %d meters per second.",
					    (int) (state.max_speed + 0.5));
			} else if ((old_state == null || old_state.state < AltosTelemetry.ao_flight_drogue) &&
				   state.state >= AltosTelemetry.ao_flight_drogue) {
				voice.speak("max height: %d meters.",
					    (int) (state.max_height + 0.5));
			}
		}
		if (old_state == null || old_state.gps_ready != state.gps_ready) {
			if (state.gps_ready)
				voice.speak("GPS ready");
			else if (old_state != null)
				voice.speak("GPS lost");
		}
		old_state = state;
	}

	class DisplayThread extends Thread {
		IdleThread	idle_thread;

		String read() throws InterruptedException { return null; }

		void close() { }

		void update(AltosState state) throws InterruptedException { }

		public void run() {
			String		line;
			AltosState	state = null;
			AltosState	old_state = null;

			idle_thread = new IdleThread();

			info_reset();
			info_finish();
			idle_thread.start();
			try {
				while ((line = read()) != null) {
					try {
						AltosTelemetry	t = new AltosTelemetry(line);
						old_state = state;
						state = new AltosState(t, state);
						update(state);
						show(state);
						tell(state, old_state);
						idle_thread.notice(state);
					} catch (ParseException pp) {
						System.out.printf("Parse error on %s\n", line);
						System.out.println("exception " + pp);
					}
				}
			} catch (InterruptedException ee) {
			} finally {
				close();
				idle_thread.interrupt();
			}
		}

		public void report() {
			if (idle_thread != null)
				idle_thread.report(true);
		}
	}

	class DeviceThread extends DisplayThread {
		AltosSerial	serial;
		LinkedBlockingQueue<String> telem;

		String read() throws InterruptedException {
			return telem.take();
		}

		void close() {
			serial.close();
			serial.remove_monitor(telem);
		}

		public DeviceThread(AltosSerial s) {
			serial = s;
			telem = new LinkedBlockingQueue<String>();
			serial.add_monitor(telem);
		}
	}

	private void ConnectToDevice() {
		altos_device	device = AltosDeviceDialog.show(AltosUI.this, "TeleDongle");

		if (device != null) {
			try {
				serial_line.open(device);
				DeviceThread thread = new DeviceThread(serial_line);
				serial_line.set_channel(AltosPreferences.channel());
				run_display(thread);
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
	}

	void DisconnectFromDevice () {
		stop_display();
	}

	String readline(FileInputStream s) throws IOException {
		int c;
		String	line = "";

		while ((c = s.read()) != -1) {
			if (c == '\r')
				continue;
			if (c == '\n') {
				return line;
			}
			line = line + (char) c;
		}
		return null;
	}

	/*
	 * Open an existing telemetry file and replay it in realtime
	 */

	class ReplayThread extends DisplayThread {
		FileInputStream	replay;
		String filename;

		ReplayThread(FileInputStream in, String name) {
			replay = in;
			filename = name;
		}

		String read() {
			try {
				return readline(replay);
			} catch (IOException ee) {
				JOptionPane.showMessageDialog(AltosUI.this,
							      filename,
							      "error reading",
							      JOptionPane.ERROR_MESSAGE);
			}
			return null;
		}

		void close () {
			try {
				replay.close();
			} catch (IOException ee) {
			}
			report();
		}

		void update(AltosState state) throws InterruptedException {
			/* Make it run in realtime after the rocket leaves the pad */
			if (state.state > AltosTelemetry.ao_flight_pad)
				Thread.sleep((int) (Math.min(state.time_change,10) * 1000));
		}
	}

	Thread		display_thread;

	private void stop_display() {
		if (display_thread != null && display_thread.isAlive())
			display_thread.interrupt();
		display_thread = null;
	}

	private void run_display(Thread thread) {
		stop_display();
		display_thread = thread;
		display_thread.start();
	}

	/*
	 * Replay a flight from telemetry data
	 */
	private void Replay() {
		JFileChooser	logfile_chooser = new JFileChooser();

		logfile_chooser.setDialogTitle("Select Telemetry File");
		logfile_chooser.setFileFilter(new FileNameExtensionFilter("Telemetry file", "telem"));
		logfile_chooser.setCurrentDirectory(AltosPreferences.logdir());
		int returnVal = logfile_chooser.showOpenDialog(AltosUI.this);

		if (returnVal == JFileChooser.APPROVE_OPTION) {
			File file = logfile_chooser.getSelectedFile();
			if (file == null)
				System.out.println("No file selected?");
			String	filename = file.getName();
			try {
				FileInputStream	replay = new FileInputStream(file);
				ReplayThread	thread = new ReplayThread(replay, filename);
				run_display(thread);
			} catch (FileNotFoundException ee) {
				JOptionPane.showMessageDialog(AltosUI.this,
							      filename,
							      "Cannot open telemetry file",
							      JOptionPane.ERROR_MESSAGE);
			}
		}
	}

	/* Connect to TeleMetrum, either directly or through
	 * a TeleDongle over the packet link
	 */
	private void SaveFlightData() {
		new AltosEeprom(AltosUI.this);
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
						DisconnectFromDevice();
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

		// Channel menu
		{
			menu = new JMenu("Channel", true);
			menu.setMnemonic(KeyEvent.VK_C);
			menubar.add(menu);
			ButtonGroup group = new ButtonGroup();

			for (int c = 0; c <= 9; c++) {
				radioitem = new JRadioButtonMenuItem(String.format("Channel %1d (%7.3fMHz)", c,
										   434.550 + c * 0.1),
								     c == AltosPreferences.channel());
				radioitem.setActionCommand(String.format("%d", c));
				radioitem.addActionListener(new ActionListener() {
						public void actionPerformed(ActionEvent e) {
							int new_channel = Integer.parseInt(e.getActionCommand());
							AltosPreferences.set_channel(new_channel);
							serial_line.set_channel(new_channel);
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
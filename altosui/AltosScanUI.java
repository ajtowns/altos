/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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
import javax.swing.event.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.util.concurrent.*;

class AltosScanResult {
	String	callsign;
	int	serial;
	int	flight;
	int	channel;
	int	telemetry;
	boolean	interrupted = false;
	
	public String toString() {
		return String.format("%-9.9s %4d %4d %2d %2d",
				     callsign, serial, flight, channel, telemetry);
	}

	public String toShortString() {
		return String.format("%s %d %d %d %d",
				     callsign, serial, flight, channel, telemetry);
	}

	public AltosScanResult(String in_callsign, int in_serial,
			       int in_flight, int in_channel, int in_telemetry) {
		callsign = in_callsign;
		serial = in_serial;
		flight = in_flight;
		channel = in_channel;
		telemetry = in_telemetry;
	}

	public boolean equals(AltosScanResult other) {
		return (callsign.equals(other.callsign) &&
			serial == other.serial &&
			flight == other.flight &&
			channel == other.channel &&
			telemetry == other.telemetry);
	}
}

class AltosScanResults extends LinkedList<AltosScanResult> implements ListModel {
	
	LinkedList<ListDataListener>	listeners = new LinkedList<ListDataListener>();

	public boolean add(AltosScanResult r) {
		for (AltosScanResult old : this)
			if (old.equals(r))
				return true;

		super.add(r);
		ListDataEvent	de = new ListDataEvent(this,
						       ListDataEvent.INTERVAL_ADDED,
						       this.size() - 2, this.size() - 1);
		for (ListDataListener l : listeners)
			l.contentsChanged(de);
		return true;
	}

	public void addListDataListener(ListDataListener l) {
		listeners.add(l);
	}
	
	public void removeListDataListener(ListDataListener l) {
		listeners.remove(l);
	}

	public AltosScanResult getElementAt(int i) {
		return this.get(i);
	}

	public int getSize() {
		return this.size();
	}
}

public class AltosScanUI
	extends JDialog
	implements ActionListener
{
	JFrame				owner;
	AltosDevice			device;
	AltosTelemetryReader		reader;
	private JList			list;
	private JLabel			channel_label;
	private JLabel			monitor_label;
	private JButton			ok_button;
	javax.swing.Timer		timer;
	AltosScanResults		results = new AltosScanResults();

	static final int[]		monitors = { Altos.ao_telemetry_split_len,
						     Altos.ao_telemetry_legacy_len };
	int				monitor;
	int				channel;

	final static int		timeout = 5 * 1000;
	TelemetryHandler		handler;
	Thread				thread;

	void scan_exception(Exception e) {
		if (e instanceof FileNotFoundException) {
			JOptionPane.showMessageDialog(owner,
						      String.format("Cannot open device \"%s\"",
								    device.toShortString()),
						      "Cannot open target device",
						      JOptionPane.ERROR_MESSAGE);
		} else if (e instanceof AltosSerialInUseException) {
			JOptionPane.showMessageDialog(owner,
						      String.format("Device \"%s\" already in use",
								    device.toShortString()),
						      "Device in use",
						      JOptionPane.ERROR_MESSAGE);
		} else if (e instanceof IOException) {
			IOException ee = (IOException) e;
			JOptionPane.showMessageDialog(owner,
						      device.toShortString(),
						      ee.getLocalizedMessage(),
						      JOptionPane.ERROR_MESSAGE);
		} else {
			JOptionPane.showMessageDialog(owner,
						      String.format("Connection to \"%s\" failed",
								    device.toShortString()),
						      "Connection Failed",
						      JOptionPane.ERROR_MESSAGE);
		}
		close();
	}

	class TelemetryHandler implements Runnable {

		public void run() {

			boolean	interrupted = false;

			try {
				for (;;) {
					try {
						AltosRecord	record = reader.read();
						if (record == null)
							break;
						if ((record.seen & AltosRecord.seen_flight) != 0) {
							AltosScanResult	result = new AltosScanResult(record.callsign,
												     record.serial,
												     record.flight,
												     channel,
												     monitor);
							results.add(result);
						}
					} catch (ParseException pp) {
						System.out.printf("Parse error: %d \"%s\"\n", pp.getErrorOffset(), pp.getMessage());
					} catch (AltosCRCException ce) {
					}
				}
			} catch (InterruptedException ee) {
				interrupted = true;
			} catch (IOException ie) {
			} finally {
				reader.close(interrupted);
			}
		}
	}

	void set_channel() {
		reader.serial.set_channel(channel);
	}

	void set_monitor() {
		reader.serial.set_telemetry(monitors[monitor]);
	}

	void next() {
		++channel;
		if (channel == 10) {
			channel = 0;
			++monitor;
			if (monitor == monitors.length)
				monitor = 0;
			set_monitor();
		}
		set_channel();
	}


	void close() {
		if (thread != null && thread.isAlive()) {
			thread.interrupt();
			try {
				thread.join();
			} catch (InterruptedException ie) {}
		}
		thread = null;
		if (timer != null)
			timer.stop();
		setVisible(false);
		dispose();
	}

	void tick_timer() {
		next();
	}

	public void actionPerformed(ActionEvent e) {
		String cmd = e.getActionCommand();

		if (cmd.equals("close")) {
			close();
		}
	}

	/* A window listener to catch closing events and tell the config code */
	class ConfigListener extends WindowAdapter {
		AltosScanUI	ui;

		public ConfigListener(AltosScanUI this_ui) {
			ui = this_ui;
		}

		public void windowClosing(WindowEvent e) {
			ui.actionPerformed(new ActionEvent(e.getSource(),
							   ActionEvent.ACTION_PERFORMED,
							   "close"));
		}
	}

	private boolean open() {
		device = AltosDeviceDialog.show(owner, Altos.product_any);
		if (device != null) {
			try {
				reader = new AltosTelemetryReader(device);
				set_channel();
				set_monitor();
				handler = new TelemetryHandler();
				thread = new Thread(handler);
				thread.start();
				return true;
			} catch (Exception e) {
				scan_exception(e);
			}
		}
		return false;
	}

	public AltosScanUI(JFrame in_owner) {

		owner = in_owner;

		if (!open())
			return;

		Container		pane = getContentPane();
		GridBagConstraints	c = new GridBagConstraints();
		Insets			i = new Insets(4,4,4,4);

		timer = new javax.swing.Timer(timeout, this);
		timer.setActionCommand("tick");
		timer.restart();

		owner = in_owner;

		pane.setLayout(new GridBagLayout());

		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		c.weightx = 1;
		c.weighty = 1;

		c.gridx = 0;
		c.gridy = 0;
		c.gridwidth = 3;
		c.anchor = GridBagConstraints.CENTER;

		list = new JList(results) {
				//Subclass JList to workaround bug 4832765, which can cause the
				//scroll pane to not let the user easily scroll up to the beginning
				//of the list.  An alternative would be to set the unitIncrement
				//of the JScrollBar to a fixed value. You wouldn't get the nice
				//aligned scrolling, but it should work.
				public int getScrollableUnitIncrement(Rectangle visibleRect,
								      int orientation,
								      int direction) {
					int row;
					if (orientation == SwingConstants.VERTICAL &&
					    direction < 0 && (row = getFirstVisibleIndex()) != -1) {
						Rectangle r = getCellBounds(row, row);
						if ((r.y == visibleRect.y) && (row != 0))  {
							Point loc = r.getLocation();
							loc.y--;
							int prevIndex = locationToIndex(loc);
							Rectangle prevR = getCellBounds(prevIndex, prevIndex);

							if (prevR == null || prevR.y >= r.y) {
								return 0;
							}
							return prevR.height;
						}
					}
					return super.getScrollableUnitIncrement(
						visibleRect, orientation, direction);
				}
			};

		list.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		list.setLayoutOrientation(JList.HORIZONTAL_WRAP);
		list.setVisibleRowCount(-1);

//		list.addMouseListener(new MouseAdapter() {
//				 public void mouseClicked(MouseEvent e) {
//					 if (e.getClickCount() == 2) {
//						 select_button.doClick(); //emulate button click
//					 }
//				 }
//			});
		JScrollPane listScroller = new JScrollPane(list);
		listScroller.setPreferredSize(new Dimension(400, 80));
		listScroller.setAlignmentX(LEFT_ALIGNMENT);

		//Create a container so that we can add a title around
		//the scroll pane.  Can't add a title directly to the
		//scroll pane because its background would be white.
		//Lay out the label and scroll pane from top to bottom.
		JPanel listPane = new JPanel();
		listPane.setLayout(new BoxLayout(listPane, BoxLayout.PAGE_AXIS));

		JLabel label = new JLabel("Select Device");
		label.setLabelFor(list);
		listPane.add(label);
		listPane.add(Box.createRigidArea(new Dimension(0,5)));
		listPane.add(listScroller);
		listPane.setBorder(BorderFactory.createEmptyBorder(10,10,10,10));

		pane.add(listPane, c);

		pack();
		setLocationRelativeTo(owner);

		addWindowListener(new ConfigListener(this));
	}
}
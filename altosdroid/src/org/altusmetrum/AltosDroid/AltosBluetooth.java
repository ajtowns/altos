/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
 * Copyright © 2012 Mike Beattie <mike@ethernal.org>
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

package org.altusmetrum.AltosDroid;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.util.Log;

import org.altusmetrum.AltosLib.*;

public class AltosBluetooth extends AltosLink {

	// Debugging
	private static final String TAG = "AltosBluetooth";
	private static final boolean D = true;

	private ConnectThread    connect_thread = null;
	private Thread           input_thread   = null;

	private BluetoothAdapter adapter;
	private BluetoothDevice  device;
	private BluetoothSocket  socket;
	private InputStream      input;
	private OutputStream     output;

	// Constructor
	public AltosBluetooth(BluetoothDevice in_device) {
		adapter = BluetoothAdapter.getDefaultAdapter();


		connect_thread = new ConnectThread(device);
		connect_thread.start();

		input_thread = new Thread(this);
		input_thread.start();
	}

	private class ConnectThread extends Thread {
		private final UUID SPP_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

		public ConnectThread(BluetoothDevice device) {
			BluetoothSocket tmp_socket = null;

			try {
				tmp_socket = device.createInsecureRfcommSocketToServiceRecord(SPP_UUID);
			} catch (IOException e) {
				e.printStackTrace();
			}
			socket = tmp_socket;
		}

		public void run() {
			if (D) Log.i(TAG, "BEGIN ConnectThread");
			setName("ConnectThread");

			// Always cancel discovery because it will slow down a connection
			adapter.cancelDiscovery();

			// Make a connection to the BluetoothSocket
			try {
				// This is a blocking call and will only return on a
				// successful connection or an exception
				socket.connect();
			} catch (IOException e) {
				// Close the socket
				try {
					socket.close();
				} catch (IOException e2) {
					if (D) Log.e(TAG, "unable to close() socket during connection failure", e2);
				}
				connection_failed();
				return;
			}

			try {
				synchronized (AltosBluetooth.this) {
					input = socket.getInputStream();
					output = socket.getOutputStream();

					// Reset the ConnectThread because we're done
					AltosBluetooth.this.notify();
					connect_thread = null;
					if (D) Log.i(TAG, "Completed connect");
				}
			} catch (Exception e) {
				if (D) Log.e(TAG, "Failed to finish connection", e);
				e.printStackTrace();
			}
		}

		public void cancel() {
			try {
				if (socket != null)
					socket.close();
			} catch (IOException e) {
				if (D) Log.e(TAG, "close() of connect socket failed", e);
			}
		}
	}

	private synchronized void wait_connected() throws InterruptedException {
		if (input == null) {
			wait();
		}
	}

	private void connection_failed() {
		if (D) Log.i(TAG, "Bluetooth Connection failed!");
	}
	
	public void print(String data) {
		byte[] bytes = data.getBytes();
		try {
			if (D) Log.i(TAG, "Entering print();");
			wait_connected();
			output.write(bytes);
			if (D) Log.i(TAG, "Writing bytes: '" + data + "'");
		} catch (IOException e) {
			connection_failed();
		} catch (InterruptedException e) {
			connection_failed();
		}
	}

	public int getchar() {
		try {
			wait_connected();
			return input.read();
		} catch (IOException e) {
			connection_failed();
		} catch (java.lang.InterruptedException e) {
			connection_failed();
		}
		return AltosLink.ERROR;
	}
			
	public void close() {
		synchronized(this) {
			if (connect_thread != null) {
				connect_thread.cancel();
				connect_thread = null;
			}
			if (input_thread != null) {
				try {
					input_thread.interrupt();
					input_thread.join();
				} catch (Exception e) {}
				input_thread = null;
			}
		}
	}


	//public void flush_output() { super.flush_output(); }

	public boolean can_cancel_reply()   { return false; }
	public boolean show_reply_timeout() { return true; }
	public void hide_reply_timeout()    { }

}

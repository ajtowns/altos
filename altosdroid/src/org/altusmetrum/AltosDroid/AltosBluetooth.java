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
//import android.os.Bundle;
import android.os.Handler;
//import android.os.Message;
import android.util.Log;

import org.altusmetrum.AltosLib.*;

public class AltosBluetooth extends AltosLink {

	// Debugging
	private static final String TAG = "AltosBluetooth";
	private static final boolean D = true;

	private ConnectThread    connect_thread = null;
	private Thread           input_thread   = null;

	private Handler          handler;

	private BluetoothAdapter adapter;
	private BluetoothDevice  device;
	private BluetoothSocket  socket;
	private InputStream      input;
	private OutputStream     output;

	// Constructor
	public AltosBluetooth(BluetoothDevice in_device, Handler in_handler) {
		adapter = BluetoothAdapter.getDefaultAdapter();
		device = in_device;
		handler = in_handler;

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
			if (D) Log.i(TAG, "ConnectThread: BEGIN");
			setName("ConnectThread");

			// Always cancel discovery because it will slow down a connection
			adapter.cancelDiscovery();

			synchronized (AltosBluetooth.this) {
				// Make a connection to the BluetoothSocket
				try {
					// This is a blocking call and will only return on a
					// successful connection or an exception
					socket.connect();

					input = socket.getInputStream();
					output = socket.getOutputStream();
				} catch (IOException e) {
					// Close the socket
					try {
						socket.close();
					} catch (IOException e2) {
						if (D) Log.e(TAG, "ConnectThread: Failed to close() socket after failed connection");
					}
					input = null;
					output = null;
					AltosBluetooth.this.notifyAll();
					handler.obtainMessage(TelemetryService.MSG_CONNECT_FAILED).sendToTarget();
					if (D) Log.e(TAG, "ConnectThread: Failed to establish connection");
					return;
				}

				// Configure the newly connected device for telemetry
				print("~\nE 0\n");
				set_monitor(false);

				// Reset the ConnectThread because we're done
				connect_thread = null;

				// Send the device name back to the Telemetry Service
				name = device.getName();
				handler.obtainMessage(TelemetryService.MSG_CONNECTED).sendToTarget();

				// Notify other waiting threads, now that we're connected
				AltosBluetooth.this.notifyAll();

				if (D) Log.i(TAG, "ConnectThread: Connect completed");
			}
		}

		public void cancel() {
			try {
				if (socket != null)
					socket.close();
			} catch (IOException e) {
				if (D) Log.e(TAG, "ConnectThread: close() of connect socket failed", e);
			}
		}
	}

	private synchronized void wait_connected() throws InterruptedException, IOException {
		if (D) Log.i(TAG, "wait_connected(): begin");
		if (input == null) {
			if (D) Log.i(TAG, "wait_connected(): waiting");
			wait();
			if (D) Log.i(TAG, "wait_connected(): wait ended..");
			if (input == null) throw new IOException();
		}
	}

	private void connection_failed() {
		if (D) Log.e(TAG, "Bluetooth Socket IO failed!");
		handler.obtainMessage(TelemetryService.MSG_DISCONNECTED).sendToTarget();
	}

	public void print(String data) {
		byte[] bytes = data.getBytes();
		if (D) Log.i(TAG, "print(): begin");
		try {
			wait_connected();
			output.write(bytes);
			if (D) Log.i(TAG, "print(): Wrote bytes: '" + data.replace('\n', '\\') + "'");
		} catch (IOException e) {
			connection_failed();
		} catch (InterruptedException e) {
			connection_failed();
		}
	}

	public int getchar() {
		if (D) Log.i(TAG, "getchar(): begin");
		try {
			wait_connected();
			if (D) Log.i(TAG, "getchar(): proceeding");
			return input.read();
		} catch (IOException e) {
			connection_failed();
		} catch (java.lang.InterruptedException e) {
			connection_failed();
		}
		return AltosLink.ERROR;
	}

	public void close() {
		if (D) Log.i(TAG, "close(): begin");
		synchronized(this) {
			if (D) Log.i(TAG, "close(): synched");

			if (connect_thread != null) {
				if (D) Log.i(TAG, "close(): stopping connect_thread");
				connect_thread.cancel();
				connect_thread = null;
			}
			if (D) Log.i(TAG, "close(): Closing socket");
			try {
				socket.close();
			} catch (IOException e) {
				if (D) Log.e(TAG, "close(): unable to close() socket");
			}
			if (input_thread != null) {
				if (D) Log.i(TAG, "close(): stopping input_thread");
				try {
					if (D) Log.i(TAG, "close(): input_thread.interrupt().....");
					input_thread.interrupt();
					if (D) Log.i(TAG, "close(): input_thread.join().....");
					input_thread.join();
				} catch (Exception e) {}
				input_thread = null;
			}
			input = null;
			output = null;
			notifyAll();
		}
	}


	//public void flush_output() { super.flush_output(); }

	public boolean can_cancel_reply()   { return false; }
	public boolean show_reply_timeout() { return true; }
	public void hide_reply_timeout()    { }

}

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

package org.altusmetrum.AltosDroid;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Method;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import org.altusmetrum.AltosLib.*;

public class AltosBluetooth extends AltosLink {

	// Debugging
	private static final String TAG = "AltosBluetooth";
	private static final boolean D = true;

	/**
	 * This thread runs while attempting to make an outgoing connection
	 * with a device. It runs straight through; the connection either
	 * succeeds or fails.
	 */

	private BluetoothAdapter	adapter;
	private ConnectThread		connect_thread;
	private BluetoothSocket		socket;
	private InputStream		input;
	private OutputStream		output;

	private class ConnectThread extends Thread {
		private final BluetoothDevice mmDevice;
		private String mSocketType;
		BluetoothSocket tmp_socket;

		public ConnectThread(BluetoothDevice device, boolean secure) {
			mmDevice = device;
			mSocketType = secure ? "Secure" : "Insecure";

			// Get a BluetoothSocket for a connection with the
			// given BluetoothDevice
			try {
				if (secure) {
					Method m = device.getClass().getMethod("createRfcommSocket", new Class[] {int.class});
					tmp_socket = (BluetoothSocket) m.invoke(device, 2);
					// tmp = device.createRfcommSocket(2);
				} else {
					Method m = device.getClass().getMethod("createInsecureRfcommSocket", new Class[] {int.class});
					tmp_socket = (BluetoothSocket) m.invoke(device, 2);
					// tmp = device.createInsecureRfcommSocket(2);
				}
			} catch (Exception e) {
				Log.e(TAG, "Socket Type: " + mSocketType + "create() failed", e);
				e.printStackTrace();
			}
		}

		public void run() {
			Log.i(TAG, "BEGIN connect_thread SocketType:" + mSocketType);
			setName("ConnectThread" + mSocketType);

			// Always cancel discovery because it will slow down a connection
			adapter.cancelDiscovery();

			// Make a connection to the BluetoothSocket
			try {
				// This is a blocking call and will only return on a
				// successful connection or an exception
				tmp_socket.connect();
			} catch (IOException e) {
				// Close the socket
				try {
					tmp_socket.close();
				} catch (IOException e2) {
					Log.e(TAG, "unable to close() " + mSocketType +
					      " socket during connection failure", e2);
				}
				connection_failed();
				return;
			}

			try {
				synchronized (AltosBluetooth.this) {
					input = tmp_socket.getInputStream();
					output = tmp_socket.getOutputStream();
					socket = tmp_socket;
					// Reset the ConnectThread because we're done
					AltosBluetooth.this.notify();
					connect_thread = null;
				}
			} catch (Exception e) {
				Log.e(TAG, "Failed to finish connection", e);
				e.printStackTrace();
			}
		}

		public void cancel() {
			try {
				if (tmp_socket != null)
					tmp_socket.close();
			} catch (IOException e) {
				Log.e(TAG, "close() of connect " + mSocketType + " socket failed", e);
			}
		}
	}

	private synchronized void wait_connected() throws InterruptedException {
		if (input == null) {
			wait();
		}
	}

	private void connection_failed() {
	}
	
	public void print(String data) {
		byte[] bytes = data.getBytes();
		try {
			wait_connected();
			output.write(bytes);
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
		}
	}

	public void flush_output() {
		super.flush_output();
		/* any local work needed to flush bluetooth? */
	}

	public boolean can_cancel_reply() {
		return false;
	}
	public boolean show_reply_timeout() {
		return true;
	}
		
	public void hide_reply_timeout() {
	}

	public AltosBluetooth(BluetoothDevice device) {
		adapter = BluetoothAdapter.getDefaultAdapter();
		connect_thread = new ConnectThread(device, true);
		connect_thread.start();
	}
}
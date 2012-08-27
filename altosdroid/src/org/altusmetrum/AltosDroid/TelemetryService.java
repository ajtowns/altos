/*
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

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.concurrent.LinkedBlockingQueue;

import android.app.Notification;
//import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
//import android.os.Bundle;
import android.os.IBinder;
import android.os.Handler;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.util.Log;
import android.widget.Toast;

import org.altusmetrum.AltosLib.*;

public class TelemetryService extends Service {

	private static final String TAG = "TelemetryService";
	private static final boolean D = true;

	static final int MSG_REGISTER_CLIENT   = 1;
	static final int MSG_UNREGISTER_CLIENT = 2;
	static final int MSG_CONNECT           = 3;
	static final int MSG_CONNECTED         = 4;
	static final int MSG_CONNECT_FAILED    = 5;
	static final int MSG_DISCONNECTED      = 6;

	public static final int STATE_NONE       = 0;
	public static final int STATE_READY      = 1;
	public static final int STATE_CONNECTING = 2;
	public static final int STATE_CONNECTED  = 3;

	// Unique Identification Number for the Notification.
	// We use it on Notification start, and to cancel it.
	private int NOTIFICATION = R.string.telemetry_service_label;
	//private NotificationManager mNM;

	ArrayList<Messenger> mClients = new ArrayList<Messenger>(); // Keeps track of all current registered clients.
	final Handler   mHandler   = new IncomingHandler(this);
	final Messenger mMessenger = new Messenger(mHandler); // Target we publish for clients to send messages to IncomingHandler.

	// Name of the connected device
	private BluetoothDevice device = null;
	private AltosBluetooth mAltosBluetooth = null;
	private int state = STATE_NONE;
	LinkedBlockingQueue<AltosLine> telem;

	// Handler of incoming messages from clients.
	static class IncomingHandler extends Handler {
		private final WeakReference<TelemetryService> service;
		IncomingHandler(TelemetryService s) { service = new WeakReference<TelemetryService>(s); }

		@Override
		public void handleMessage(Message msg) {
			TelemetryService s = service.get();
			switch (msg.what) {
			case MSG_REGISTER_CLIENT:
				s.mClients.add(msg.replyTo);
				try {
					msg.replyTo.send(Message.obtain(null, AltosDroid.MSG_STATE_CHANGE, s.state, -1));
				} catch (RemoteException e) {
					s.mClients.remove(msg.replyTo);
				}
				if (D) Log.d(TAG, "Client bound to service");
				break;
			case MSG_UNREGISTER_CLIENT:
				s.mClients.remove(msg.replyTo);
				if (D) Log.d(TAG, "Client unbound from service");
				break;
			case MSG_CONNECT:
				if (D) Log.d(TAG, "Connect command received");
				s.device = (BluetoothDevice) msg.obj;
				s.startAltosBluetooth();
				break;
			case MSG_CONNECTED:
				if (D) Log.d(TAG, "Connected to device");
				s.sendMessageToClients(Message.obtain(null, AltosDroid.MSG_DEVNAME, s.device.getName()));
				s.setState(STATE_CONNECTED);
				s.mAltosBluetooth.add_monitor(s.telem);
				break;
			case MSG_CONNECT_FAILED:
				if (D) Log.d(TAG, "Connection failed... retrying");
				s.startAltosBluetooth();
				break;
			case MSG_DISCONNECTED:
				if (D) Log.d(TAG, "Disconnected from " + s.device.getName());
				s.stopAltosBluetooth();
				break;
			default:
				super.handleMessage(msg);
			}
		}
	}

	private void sendMessageToClients(Message m) {
		for (int i=mClients.size()-1; i>=0; i--) {
			try {
				mClients.get(i).send(m);
			} catch (RemoteException e) {
				mClients.remove(i);
			}
		}
	}

	private void stopAltosBluetooth() {
		if (D) Log.i(TAG, "Stopping BT");
		setState(STATE_READY);
		if (mAltosBluetooth != null) {
			if (D) Log.i(TAG, "Closing AltosBluetooth");
			mAltosBluetooth.close();
			mAltosBluetooth = null;
		}
		device = null;
		telem.clear();
	}

	private void startAltosBluetooth() {
		if (mAltosBluetooth == null) {
			if (D) Log.i(TAG, "Connecting to " + device.getName());
			mAltosBluetooth = new AltosBluetooth(device, mHandler);
			setState(STATE_CONNECTING);
		} else {
			stopAltosBluetooth();
			mHandler.sendMessageDelayed(Message.obtain(null, MSG_CONNECT, device), 1000);
		}
	}

	private synchronized void setState(int s) {
		if (D) Log.d(TAG, "setState() " + state + " -> " + s);
		state = s;

		sendMessageToClients(Message.obtain(null, AltosDroid.MSG_STATE_CHANGE, state, -1));
	}

	@Override
	public void onCreate() {
		// Create a reference to the NotificationManager so that we can update our notifcation text later
		//mNM = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);

		telem = new LinkedBlockingQueue<AltosLine>();
		setState(STATE_READY);
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		Log.i("TelemetryService", "Received start id " + startId + ": " + intent);

		CharSequence text = getText(R.string.telemetry_service_started);

		// Create notification to be displayed while the service runs
		Notification notification = new Notification(R.drawable.am_status_c, text, 0);

		// The PendingIntent to launch our activity if the user selects this notification
		PendingIntent contentIntent = PendingIntent.getActivity(this, 0,
				new Intent(this, AltosDroid.class), 0);

		// Set the info for the views that show in the notification panel.
		notification.setLatestEventInfo(this, getText(R.string.telemetry_service_label), text, contentIntent);

		// Set the notification to be in the "Ongoing" section.
		notification.flags |= Notification.FLAG_ONGOING_EVENT;

		// Move us into the foreground.
		startForeground(NOTIFICATION, notification);

		// We want this service to continue running until it is explicitly
		// stopped, so return sticky.
		return START_STICKY;
	}

	@Override
	public void onDestroy() {

		// Stop the bluetooth Comms threads
		stopAltosBluetooth();

		// Demote us from the foreground, and cancel the persistent notification.
		stopForeground(true);

		// Tell the user we stopped.
		Toast.makeText(this, R.string.telemetry_service_stopped, Toast.LENGTH_SHORT).show();
	}

	@Override
	public IBinder onBind(Intent intent) {
		return mMessenger.getBinder();
	}


}

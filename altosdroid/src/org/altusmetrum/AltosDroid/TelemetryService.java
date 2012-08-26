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
import android.os.IBinder;
import android.os.Handler;
import android.os.Message;
import android.os.Messenger;
import android.util.Log;
import android.widget.Toast;

// Need the following import to get access to the app resources, since this
// class is in a sub-package.
//import org.altusmetrum.AltosDroid.R;

import org.altusmetrum.AltosLib.*;

public class TelemetryService extends Service {

	private static final String TAG = "TelemetryService";
	private static final boolean D = true;

	static final int MSG_REGISTER_CLIENT   = 1;
	static final int MSG_UNREGISTER_CLIENT = 2;
	static final int MSG_CONNECT           = 3;
	static final int MSG_CONNECTED         = 4;

	public static final int STATE_NONE       = 0;
	public static final int STATE_READY      = 1;
	public static final int STATE_CONNECTING = 2;
	public static final int STATE_CONNECTED  = 3;

	// Unique Identification Number for the Notification.
	// We use it on Notification start, and to cancel it.
	private int NOTIFICATION = R.string.telemetry_service_label;
	//private NotificationManager mNM;

	ArrayList<Messenger> mClients = new ArrayList<Messenger>(); // Keeps track of all current registered clients.
	final Messenger mMessenger = new Messenger(new IncomingHandler()); // Target we publish for clients to send messages to IncomingHandler.

	// Name of the connected device
	private String mConnectedDeviceName = null;
	private AltosBluetooth mAltosBluetooth = null;

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
				if (D) Log.d(TAG, "Client bound to service");
				break;
			case MSG_UNREGISTER_CLIENT:
				s.mClients.remove(msg.replyTo);
				if (D) Log.d(TAG, "Client unbound from service");
				break;
			case MSG_CONNECT:
				if (D) Log.d(TAG, "Connect command received");
				s.startAltosBluetooth((BluetoothDevice) msg.obj);
				break;
			case MSG_CONNECTED:
				if (D) Log.d(TAG, "Connected to device");
				break;
			default:
				super.handleMessage(msg);
			}
		}
	}

	private void stopAltosBluetooth() {
		if (mAltosBluetooth != null) {
			mAltosBluetooth.close();
			mAltosBluetooth = null;
		}
		telem.clear();
	}

	private void startAltosBluetooth(BluetoothDevice d) {
		mAltosBluetooth = new AltosBluetooth(d);
		mAltosBluetooth.add_monitor(telem);
	}

	@Override
	public void onCreate() {
		// Create a reference to the NotificationManager so that we can update our notifcation text later
		//mNM = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);

		telem = new LinkedBlockingQueue<AltosLine>();
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

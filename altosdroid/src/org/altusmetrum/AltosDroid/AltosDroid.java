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

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.content.Context;
import android.content.ComponentName;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.speech.tts.TextToSpeech;
import android.speech.tts.TextToSpeech.OnInitListener;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.Window;
import android.widget.TextView;
import android.widget.Toast;

/**
 * This is the main Activity that displays the current chat session.
 */
public class AltosDroid extends Activity {
	// Debugging
	private static final String TAG = "AltosDroid";
	private static final boolean D = true;

	// Message types received by our Handler
	public static final int MSG_STATE_CHANGE    = 1;
	public static final int MSG_DEVNAME         = 2;
	public static final int MSG_TOAST           = 3;
	public static final int MSG_DEVCONFIG       = 4;
	public static final int MSG_TELEMETRY       = 5;

	// Intent request codes
	private static final int REQUEST_CONNECT_DEVICE = 1;
	private static final int REQUEST_ENABLE_BT      = 2;

	// Layout Views
	private TextView mTitle;
	private TextView mSerialView;

	// Service
	private boolean mIsBound   = false;
	private Messenger mService = null;
	final Messenger mMessenger = new Messenger(new IncomingHandler(this));

	// Name of the connected device
	private String mConnectedDeviceName = null;
	// Local Bluetooth adapter
	private BluetoothAdapter mBluetoothAdapter = null;

	// Text to Speech
	private TextToSpeech tts    = null;
	private boolean tts_enabled = false;

	// The Handler that gets information back from the Telemetry Service
	static class IncomingHandler extends Handler {
		private final WeakReference<AltosDroid> mAltosDroid;
		IncomingHandler(AltosDroid ad) { mAltosDroid = new WeakReference<AltosDroid>(ad); }

		@Override
		public void handleMessage(Message msg) {
			AltosDroid ad = mAltosDroid.get();
			switch (msg.what) {
			case MSG_STATE_CHANGE:
				if(D) Log.d(TAG, "MSG_STATE_CHANGE: " + msg.arg1);
				switch (msg.arg1) {
				case TelemetryService.STATE_CONNECTED:
					ad.mTitle.setText(R.string.title_connected_to);
					ad.mTitle.append(ad.mConnectedDeviceName);
					ad.mSerialView.setText("");
					break;
				case TelemetryService.STATE_CONNECTING:
					ad.mTitle.setText(R.string.title_connecting);
					break;
				case TelemetryService.STATE_READY:
				case TelemetryService.STATE_NONE:
					ad.mTitle.setText(R.string.title_not_connected);
					break;
				}
				break;
			case MSG_DEVCONFIG:
			case MSG_TELEMETRY:
				//byte[] buf = (byte[]) msg.obj;
				// construct a string from the buffer
				//String telem = new String(buf);
				//ad.mSerialView.append(telem);
				break;
			case MSG_DEVNAME:
				// save the connected device's name
				ad.mConnectedDeviceName = (String) msg.obj;
				if (ad.mConnectedDeviceName != null)
					Toast.makeText(ad.getApplicationContext(), "Connected to "
							+ ad.mConnectedDeviceName, Toast.LENGTH_SHORT).show();
				break;
			case MSG_TOAST:
				Toast.makeText(
						ad.getApplicationContext(),
						(String) msg.obj,
						Toast.LENGTH_SHORT).show();
				break;
			}
		}
	};


	private ServiceConnection mConnection = new ServiceConnection() {
		public void onServiceConnected(ComponentName className, IBinder service) {
			mService = new Messenger(service);
			try {
				Message msg = Message.obtain(null, TelemetryService.MSG_REGISTER_CLIENT);
				msg.replyTo = mMessenger;
				mService.send(msg);
			} catch (RemoteException e) {
				// In this case the service has crashed before we could even do anything with it
			}
		}

		public void onServiceDisconnected(ComponentName className) {
			// This is called when the connection with the service has been unexpectedly disconnected - process crashed.
			mService = null;
		}
	};


	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		if(D) Log.e(TAG, "+++ ON CREATE +++");

		// Set up the window layout
		requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
		setContentView(R.layout.main);
		getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.custom_title);

		// Set up the custom title
		mTitle = (TextView) findViewById(R.id.title_left_text);
		mTitle.setText(R.string.app_name);
		mTitle = (TextView) findViewById(R.id.title_right_text);

		// Set up the temporary Text View
		mSerialView = (TextView) findViewById(R.id.in);
		mSerialView.setMovementMethod(new ScrollingMovementMethod());
		mSerialView.setClickable(false);
		mSerialView.setLongClickable(false);

		// Get local Bluetooth adapter
		mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

		// If the adapter is null, then Bluetooth is not supported
		if (mBluetoothAdapter == null) {
			Toast.makeText(this, "Bluetooth is not available", Toast.LENGTH_LONG).show();
			finish();
			return;
		}

		// Enable Text to Speech
		tts = new TextToSpeech(this, new OnInitListener() {
			public void onInit(int status) {
				if (status == TextToSpeech.SUCCESS) tts_enabled = true;
				if (tts_enabled) tts.speak("AltosDroid ready", TextToSpeech.QUEUE_ADD, null );
			}
		});

		// Start Telemetry Service
		startService(new Intent(AltosDroid.this, TelemetryService.class));

		doBindService();
	}

	@Override
	public void onStart() {
		super.onStart();
		if(D) Log.e(TAG, "++ ON START ++");

		if (!mBluetoothAdapter.isEnabled()) {
			Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
			startActivityForResult(enableIntent, REQUEST_ENABLE_BT);
		}
	}

	@Override
	public synchronized void onResume() {
		super.onResume();
		if(D) Log.e(TAG, "+ ON RESUME +");

	}

	@Override
	public synchronized void onPause() {
		super.onPause();
		if(D) Log.e(TAG, "- ON PAUSE -");
	}

	@Override
	public void onStop() {
		super.onStop();
		if(D) Log.e(TAG, "-- ON STOP --");
	}

	@Override
	public void onDestroy() {
		super.onDestroy();

		doUnbindService();

		if (tts != null) tts.shutdown();

		if(D) Log.e(TAG, "--- ON DESTROY ---");
	}




	public void onActivityResult(int requestCode, int resultCode, Intent data) {
		if(D) Log.d(TAG, "onActivityResult " + resultCode);
		switch (requestCode) {
		case REQUEST_CONNECT_DEVICE:
			// When DeviceListActivity returns with a device to connect to
			if (resultCode == Activity.RESULT_OK) {
				connectDevice(data);
			}
			break;
		case REQUEST_ENABLE_BT:
			// When the request to enable Bluetooth returns
			if (resultCode == Activity.RESULT_OK) {
				// Bluetooth is now enabled, so set up a chat session
				//setupChat();
			} else {
				// User did not enable Bluetooth or an error occured
				Log.e(TAG, "BT not enabled");
				stopService(new Intent(AltosDroid.this, TelemetryService.class));
				Toast.makeText(this, R.string.bt_not_enabled, Toast.LENGTH_SHORT).show();
				finish();
			}
			break;
		}
	}

	private void connectDevice(Intent data) {
		// Get the device MAC address
		String address = data.getExtras().getString(DeviceListActivity.EXTRA_DEVICE_ADDRESS);
		// Get the BLuetoothDevice object
		BluetoothDevice device = mBluetoothAdapter.getRemoteDevice(address);
		// Attempt to connect to the device
		try {
			if (D) Log.d(TAG, "Connecting to " + device.getName());
			mService.send(Message.obtain(null, TelemetryService.MSG_CONNECT, device));
		} catch (RemoteException e) {
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.option_menu, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		Intent serverIntent = null;
		switch (item.getItemId()) {
		case R.id.connect_scan:
			// Launch the DeviceListActivity to see devices and do scan
			serverIntent = new Intent(this, DeviceListActivity.class);
			startActivityForResult(serverIntent, REQUEST_CONNECT_DEVICE);
			return true;
		}
		return false;
	}


	void doBindService() {
		bindService(new Intent(this, TelemetryService.class), mConnection, Context.BIND_AUTO_CREATE);
		mIsBound = true;
	}

	void doUnbindService() {
		if (mIsBound) {
			// If we have received the service, and hence registered with it, then now is the time to unregister.
			if (mService != null) {
				try {
					Message msg = Message.obtain(null, TelemetryService.MSG_UNREGISTER_CLIENT);
					msg.replyTo = mMessenger;
					mService.send(msg);
				} catch (RemoteException e) {
					// There is nothing special we need to do if the service has crashed.
				}
			}
			// Detach our existing connection.
			unbindService(mConnection);
			mIsBound = false;
		}
	}

}

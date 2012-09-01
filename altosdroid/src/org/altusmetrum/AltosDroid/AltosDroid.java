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
import android.content.DialogInterface;
import android.os.IBinder;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.Window;
import android.widget.TextView;
import android.widget.Toast;
import android.app.AlertDialog;

import org.altusmetrum.AltosLib.*;

/**
 * This is the main Activity that displays the current chat session.
 */
public class AltosDroid extends Activity {
	// Debugging
	private static final String TAG = "AltosDroid";
	private static final boolean D = true;

	// Message types received by our Handler
	public static final int MSG_STATE_CHANGE    = 1;
	public static final int MSG_TELEMETRY       = 2;

	// Intent request codes
	private static final int REQUEST_CONNECT_DEVICE = 1;
	private static final int REQUEST_ENABLE_BT      = 2;

	// Layout Views
	private TextView mTitle;

	// Flight state values
	private TextView mCallsignView;
	private TextView mRSSIView;
	private TextView mSerialView;
	private TextView mFlightView;
	private TextView mStateView;
	private TextView mSpeedView;
	private TextView mAccelView;
	private TextView mRangeView;
	private TextView mHeightView;
	private TextView mElevationView;
	private TextView mBearingView;
	private TextView mLatitudeView;
	private TextView mLongitudeView;

	// Generic field for extras at the bottom
	private TextView mTextView;

	// Service
	private boolean mIsBound   = false;
	private Messenger mService = null;
	final Messenger mMessenger = new Messenger(new IncomingHandler(this));

	// TeleBT Config data
	private AltosConfigData mConfigData = null;
	// Local Bluetooth adapter
	private BluetoothAdapter mBluetoothAdapter = null;

	// Text to Speech
	private AltosVoice mAltosVoice = null;

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
					ad.mConfigData = (AltosConfigData) msg.obj;
					String str = String.format(" %s S/N: %d", ad.mConfigData.product, ad.mConfigData.serial);
					ad.mTitle.setText(R.string.title_connected_to);
					ad.mTitle.append(str);
					Toast.makeText(ad.getApplicationContext(), "Connected to " + str, Toast.LENGTH_SHORT).show();
					ad.mAltosVoice.speak("Connected");
					//TEST!
					ad.mTextView.setText(Dumper.dump(ad.mConfigData));
					break;
				case TelemetryService.STATE_CONNECTING:
					ad.mTitle.setText(R.string.title_connecting);
					break;
				case TelemetryService.STATE_READY:
				case TelemetryService.STATE_NONE:
					ad.mConfigData = null;
					ad.mTitle.setText(R.string.title_not_connected);
					ad.mTextView.setText("");
					break;
				}
				break;
			case MSG_TELEMETRY:
				ad.update_ui((AltosState) msg.obj);
				// TEST!
				ad.mTextView.setText(Dumper.dump(msg.obj));
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

	void update_ui(AltosState state) {
		mCallsignView.setText(state.data.callsign);
		mRSSIView.setText(String.format("%d", state.data.rssi));
		mSerialView.setText(String.format("%d", state.data.serial));
		mFlightView.setText(String.format("%d", state.data.flight));
		mStateView.setText(state.data.state());
		double speed = state.speed;
		if (!state.ascent)
			speed = state.baro_speed;
		mSpeedView.setText(String.format("%6.0f m/s", speed));
		mAccelView.setText(String.format("%6.0f m/s²", state.acceleration));
		mRangeView.setText(String.format("%6.0f m", state.range));
		mHeightView.setText(String.format("%6.0f m", state.height));
		mElevationView.setText(String.format("%3.0f°", state.elevation));
		if (state.from_pad != null)
			mBearingView.setText(String.format("%3.0f°", state.from_pad.bearing));
		mLatitudeView.setText(pos(state.gps.lat, "N", "S"));
		mLongitudeView.setText(pos(state.gps.lon, "W", "E"));

		mAltosVoice.tell(state);
	}

	String pos(double p, String pos, String neg) {
		String	h = pos;
		if (p < 0) {
			h = neg;
			p = -p;
		}
		int deg = (int) Math.floor(p);
		double min = (p - Math.floor(p)) * 60.0;
		return String.format("%d° %9.6f\" %s", deg, min, h);
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		if(D) Log.e(TAG, "+++ ON CREATE +++");

		// Get local Bluetooth adapter
		mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

		// If the adapter is null, then Bluetooth is not supported
		if (mBluetoothAdapter == null) {
			Toast.makeText(this, "Bluetooth is not available", Toast.LENGTH_LONG).show();
			finish();
			return;
		}

		// Set up the window layout
		requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
		//setContentView(R.layout.main);
		setContentView(R.layout.altosdroid);
		getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.custom_title);

		// Set up the custom title
		mTitle = (TextView) findViewById(R.id.title_left_text);
		mTitle.setText(R.string.app_name);
		mTitle = (TextView) findViewById(R.id.title_right_text);

		// Set up the temporary Text View
		mTextView = (TextView) findViewById(R.id.text);
		mTextView.setMovementMethod(new ScrollingMovementMethod());
		mTextView.setClickable(false);
		mTextView.setLongClickable(false);

		mCallsignView  = (TextView) findViewById(R.id.callsign_value);
		mRSSIView      = (TextView) findViewById(R.id.rssi_value);
		mSerialView    = (TextView) findViewById(R.id.serial_value);
		mFlightView    = (TextView) findViewById(R.id.flight_value);
		mStateView     = (TextView) findViewById(R.id.state_value);
		mSpeedView     = (TextView) findViewById(R.id.speed_value);
		mAccelView     = (TextView) findViewById(R.id.accel_value);
		mRangeView     = (TextView) findViewById(R.id.range_value);
		mHeightView    = (TextView) findViewById(R.id.height_value);
		mElevationView = (TextView) findViewById(R.id.elevation_value);
		mBearingView   = (TextView) findViewById(R.id.bearing_value);
		mLatitudeView  = (TextView) findViewById(R.id.latitude_value);
		mLongitudeView = (TextView) findViewById(R.id.longitude_value);

		mAltosVoice = new AltosVoice(this);
	}

	@Override
	public void onStart() {
		super.onStart();
		if(D) Log.e(TAG, "++ ON START ++");

		if (!mBluetoothAdapter.isEnabled()) {
			Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
			startActivityForResult(enableIntent, REQUEST_ENABLE_BT);
		}

		// Start Telemetry Service
		startService(new Intent(AltosDroid.this, TelemetryService.class));

		doBindService();
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

		doUnbindService();
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
		if(D) Log.e(TAG, "--- ON DESTROY ---");

		mAltosVoice.stop();
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

	void setFrequency(double freq) {
		try {
			mService.send(Message.obtain(null, TelemetryService.MSG_SETFREQUENCY, freq));
		} catch (RemoteException e) {
		}
	}

	void setFrequency(String freq) {
		try {
			setFrequency (Double.parseDouble(freq.substring(11, 17)));
		} catch (NumberFormatException e) {
		}
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
		case R.id.select_freq:
			// Set the TBT radio frequency

			final String[] frequencies = {
				"Channel 0 (434.550MHz)",
				"Channel 1 (434.650MHz)",
				"Channel 2 (434.750MHz)",
				"Channel 3 (434.850MHz)",
				"Channel 4 (434.950MHz)",
				"Channel 5 (435.050MHz)",
				"Channel 6 (435.150MHz)",
				"Channel 7 (435.250MHz)",
				"Channel 8 (435.350MHz)",
				"Channel 9 (435.450MHz)"
			};

			AlertDialog.Builder builder = new AlertDialog.Builder(this);
			builder.setTitle("Pick a frequency");
			builder.setItems(frequencies,
					 new DialogInterface.OnClickListener() {
						 public void onClick(DialogInterface dialog, int item) {
							 setFrequency(frequencies[item]);
						 }
					 });
			AlertDialog alert = builder.create();
			alert.show();
			return true;
		}
		return false;
	}

}

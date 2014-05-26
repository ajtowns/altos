/*
 * Copyright © 2012-2013 Mike Beattie <mike@ethernal.org>
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
import java.util.Timer;
import java.util.TimerTask;

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
import android.support.v4.app.FragmentActivity;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.Window;
import android.widget.TabHost;
import android.widget.TextView;
import android.widget.Toast;
import android.app.AlertDialog;
import android.location.Location;

import org.altusmetrum.altoslib_4.*;

public class AltosDroid extends FragmentActivity {
	// Debugging
	private static final String TAG = "AltosDroid";
	private static final boolean D = true;

	// Message types received by our Handler
	public static final int MSG_STATE_CHANGE    = 1;
	public static final int MSG_TELEMETRY       = 2;
	public static final int MSG_UPDATE_AGE      = 3;
	public static final int MSG_LOCATION	    = 4;
	public static final int MSG_CRC_ERROR	    = 5;

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
	private TextView mAgeView;

	// field to display the version at the bottom of the screen
	private TextView mVersion;

	// Tabs
	TabHost         mTabHost;
	AltosViewPager  mViewPager;
	TabsAdapter     mTabsAdapter;
	ArrayList<AltosDroidTab> mTabs = new ArrayList<AltosDroidTab>();
	int             tabHeight;

	// Timer and Saved flight state for Age calculation
	private Timer timer = new Timer();
	AltosState saved_state;
	Location saved_location;

	// Service
	private boolean mIsBound   = false;
	private Messenger mService = null;
	final Messenger mMessenger = new Messenger(new IncomingHandler(this));

	// Preferences
	private AltosDroidPreferences prefs = null;

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
					break;
				case TelemetryService.STATE_CONNECTING:
					ad.mTitle.setText(R.string.title_connecting);
					break;
				case TelemetryService.STATE_READY:
				case TelemetryService.STATE_NONE:
					ad.mConfigData = null;
					ad.mTitle.setText(R.string.title_not_connected);
					break;
				}
				break;
			case MSG_TELEMETRY:
				ad.update_ui((AltosState) msg.obj);
				break;
			case MSG_LOCATION:
				ad.set_location((Location) msg.obj);
				break;
			case MSG_CRC_ERROR:
			case MSG_UPDATE_AGE:
				if (ad.saved_state != null) {
					ad.mAgeView.setText(String.format("%d", (System.currentTimeMillis() - ad.saved_state.received_time + 500) / 1000));
				}
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

	public void registerTab(AltosDroidTab mTab) {
		mTabs.add(mTab);
	}

	public void unregisterTab(AltosDroidTab mTab) {
		mTabs.remove(mTab);
	}

	void set_location(Location location) {
		saved_location = location;
		update_ui(saved_state);
	}

	void update_ui(AltosState state) {
		if (state != null && saved_state != null) {
			if (saved_state.state != state.state) {
				String currentTab = mTabHost.getCurrentTabTag();
				switch (state.state) {
				case AltosLib.ao_flight_boost:
					if (currentTab.equals("pad")) mTabHost.setCurrentTabByTag("ascent");
					break;
				case AltosLib.ao_flight_drogue:
					if (currentTab.equals("ascent")) mTabHost.setCurrentTabByTag("descent");
					break;
				case AltosLib.ao_flight_landed:
					if (currentTab.equals("descent")) mTabHost.setCurrentTabByTag("landed");
					break;
				}
			}
		}
		saved_state = state;

		AltosGreatCircle from_receiver = null;

		if (state != null && saved_location != null && state.gps != null && state.gps.locked) {
			double altitude = 0;
			if (saved_location.hasAltitude())
				altitude = saved_location.getAltitude();
			from_receiver = new AltosGreatCircle(saved_location.getLatitude(),
							     saved_location.getLongitude(),
							     altitude,
							     state.gps.lat,
							     state.gps.lon,
							     state.gps.alt);
		}

		if (state != null) {
			mCallsignView.setText(state.callsign);
			mSerialView.setText(String.format("%d", state.serial));
			mFlightView.setText(String.format("%d", state.flight));
			mStateView.setText(state.state_name());
			mRSSIView.setText(String.format("%d", state.rssi));
		}

		for (AltosDroidTab mTab : mTabs)
			mTab.update_ui(state, from_receiver, saved_location);

		if (state != null)
			mAltosVoice.tell(state);
	}

	private void onTimerTick() {
		try {
			mMessenger.send(Message.obtain(null, MSG_UPDATE_AGE));
		} catch (RemoteException e) {
		}
	}

	static String pos(double p, String pos, String neg) {
		String	h = pos;
		if (p == AltosLib.MISSING)
			return "";
		if (p < 0) {
			h = neg;
			p = -p;
		}
		int deg = (int) Math.floor(p);
		double min = (p - Math.floor(p)) * 60.0;
		return String.format("%d°%9.4f\" %s", deg, min, h);
	}

	static String number(String format, double value) {
		if (value == AltosLib.MISSING)
			return "";
		return String.format(format, value);
	}

	static String integer(String format, int value) {
		if (value == AltosLib.MISSING)
			return "";
		return String.format(format, value);
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

		// Initialise preferences
		prefs = new AltosDroidPreferences(this);
		AltosPreferences.init(prefs);

		// Set up the window layout
		requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
		setContentView(R.layout.altosdroid);
		getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.custom_title);

		// Create the Tabs and ViewPager
		mTabHost = (TabHost)findViewById(android.R.id.tabhost);
		mTabHost.setup();

		mViewPager = (AltosViewPager)findViewById(R.id.pager);
		mViewPager.setOffscreenPageLimit(4);

		mTabsAdapter = new TabsAdapter(this, mTabHost, mViewPager);

		mTabsAdapter.addTab(mTabHost.newTabSpec("pad").setIndicator("Pad"), TabPad.class, null);
		mTabsAdapter.addTab(mTabHost.newTabSpec("ascent").setIndicator("Ascent"), TabAscent.class, null);
		mTabsAdapter.addTab(mTabHost.newTabSpec("descent").setIndicator("Descent"), TabDescent.class, null);
		mTabsAdapter.addTab(mTabHost.newTabSpec("landed").setIndicator("Landed"), TabLanded.class, null);
		mTabsAdapter.addTab(mTabHost.newTabSpec("map").setIndicator("Map"), TabMap.class, null);


		// Scale the size of the Tab bar for different screen densities
		// This probably won't be needed when we start supporting ICS+ tabs.
		DisplayMetrics metrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(metrics);
		int density = metrics.densityDpi;

		if (density==DisplayMetrics.DENSITY_XHIGH)
			tabHeight = 65;
		else if (density==DisplayMetrics.DENSITY_HIGH)
			tabHeight = 45;
		else if (density==DisplayMetrics.DENSITY_MEDIUM)
			tabHeight = 35;
		else if (density==DisplayMetrics.DENSITY_LOW)
			tabHeight = 25;
		else
			tabHeight = 65;

		for (int i = 0; i < 5; i++)
			mTabHost.getTabWidget().getChildAt(i).getLayoutParams().height = tabHeight;


		// Set up the custom title
		mTitle = (TextView) findViewById(R.id.title_left_text);
		mTitle.setText(R.string.app_name);
		mTitle = (TextView) findViewById(R.id.title_right_text);

		// Display the Version
		mVersion = (TextView) findViewById(R.id.version);
		mVersion.setText("Version: " + BuildInfo.version +
		                 "  Built: " + BuildInfo.builddate + " " + BuildInfo.buildtime + " " + BuildInfo.buildtz +
		                 "  (" + BuildInfo.branch + "-" + BuildInfo.commitnum + "-" + BuildInfo.commithash + ")");

		mCallsignView  = (TextView) findViewById(R.id.callsign_value);
		mRSSIView      = (TextView) findViewById(R.id.rssi_value);
		mSerialView    = (TextView) findViewById(R.id.serial_value);
		mFlightView    = (TextView) findViewById(R.id.flight_value);
		mStateView     = (TextView) findViewById(R.id.state_value);
		mAgeView       = (TextView) findViewById(R.id.age_value);

		timer.scheduleAtFixedRate(new TimerTask(){ public void run() {onTimerTick();}}, 1000L, 100L);

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

		if (mAltosVoice != null) mAltosVoice.stop();
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

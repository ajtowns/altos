package org.altusmetrum.AltosDroid;

import org.altusmetrum.altoslib_4.*;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Environment;
import android.util.Log;

public class TelemetryLogger {
	private static final String TAG = "TelemetryLogger";
	private static final boolean D = true;

	private Context   context = null;
	private AltosLink link    = null;
	private AltosLog  logger  = null;

	private BroadcastReceiver mExternalStorageReceiver;

	public TelemetryLogger(Context in_context, AltosLink in_link) {
		context = in_context;
		link    = in_link;

		startWatchingExternalStorage();
	}

	public void stop() {
		stopWatchingExternalStorage();
		close();
	}

	private void close() {
		if (logger != null) {
			if (D) Log.d(TAG, "Shutting down Telemetry Logging");
			logger.close();
			logger = null;
		}
	}
	
	void handleExternalStorageState() {
		String state = Environment.getExternalStorageState();
		if (Environment.MEDIA_MOUNTED.equals(state)) {
			if (logger == null) {
				if (D) Log.d(TAG, "Starting up Telemetry Logging");
				logger = new AltosLog(link);
			}
		} else {
			if (D) Log.d(TAG, "External Storage not present - stopping");
			close();
		}
	}

	void startWatchingExternalStorage() {
		mExternalStorageReceiver = new BroadcastReceiver() {
			@Override
			public void onReceive(Context context, Intent intent) {
				handleExternalStorageState();
			}
		};
		IntentFilter filter = new IntentFilter();
		filter.addAction(Intent.ACTION_MEDIA_MOUNTED);
		filter.addAction(Intent.ACTION_MEDIA_REMOVED);
		context.registerReceiver(mExternalStorageReceiver, filter);
		handleExternalStorageState();
	}

	void stopWatchingExternalStorage() {
		context.unregisterReceiver(mExternalStorageReceiver);
	}

}

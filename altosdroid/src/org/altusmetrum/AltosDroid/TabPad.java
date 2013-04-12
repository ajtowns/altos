/*
 * Copyright © 2013 Mike Beattie <mike@ethernal.org>
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

import org.altusmetrum.altoslib_1.*;

import android.app.Activity;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;
import android.location.Location;

public class TabPad extends Fragment implements AltosDroidTab {
	AltosDroid mAltosDroid;

	private TextView mBatteryVoltageView;
	private GoNoGoLights mBatteryLights;
	private TextView mApogeeVoltageView;
	private GoNoGoLights mApogeeLights;
	private TextView mMainVoltageView;
	private GoNoGoLights mMainLights;
	private TextView mDataLoggingView;
	private GoNoGoLights mDataLoggingLights;
	private TextView mGPSLockedView;
	private GoNoGoLights mGPSLockedLights;
	private TextView mGPSReadyView;
	private GoNoGoLights mGPSReadyLights;
	private TextView mPadLatitudeView;
	private TextView mPadLongitudeView;
	private TextView mPadAltitudeView;

	@Override
	public void onAttach(Activity activity) {
		super.onAttach(activity);
		mAltosDroid = (AltosDroid) activity;
		mAltosDroid.registerTab(this);
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.tab_pad, container, false);
		mBatteryVoltageView = (TextView) v.findViewById(R.id.battery_voltage_value);
		mBatteryLights = new GoNoGoLights((ImageView) v.findViewById(R.id.battery_redled),
		                                  (ImageView) v.findViewById(R.id.battery_greenled),
		                                  getResources());

		mApogeeVoltageView = (TextView) v.findViewById(R.id.apogee_voltage_value);
		mApogeeLights = new GoNoGoLights((ImageView) v.findViewById(R.id.apogee_redled),
		                                 (ImageView) v.findViewById(R.id.apogee_greenled),
		                                 getResources());

		mMainVoltageView = (TextView) v.findViewById(R.id.main_voltage_value);
		mMainLights = new GoNoGoLights((ImageView) v.findViewById(R.id.main_redled),
		                               (ImageView) v.findViewById(R.id.main_greenled),
		                               getResources());

		mDataLoggingView = (TextView) v.findViewById(R.id.logging_value);
		mDataLoggingLights = new GoNoGoLights((ImageView) v.findViewById(R.id.logging_redled),
		                                      (ImageView) v.findViewById(R.id.logging_greenled),
		                                      getResources());

		mGPSLockedView = (TextView) v.findViewById(R.id.gps_locked_value);
		mGPSLockedLights = new GoNoGoLights((ImageView) v.findViewById(R.id.gps_locked_redled),
		                                    (ImageView) v.findViewById(R.id.gps_locked_greenled),
		                                    getResources());

		mGPSReadyView = (TextView) v.findViewById(R.id.gps_ready_value);
		mGPSReadyLights = new GoNoGoLights((ImageView) v.findViewById(R.id.gps_ready_redled),
		                                   (ImageView) v.findViewById(R.id.gps_ready_greenled),
		                                   getResources());

		mPadLatitudeView = (TextView) v.findViewById(R.id.pad_lat_value);
		mPadLongitudeView = (TextView) v.findViewById(R.id.pad_lon_value);
		mPadAltitudeView = (TextView) v.findViewById(R.id.pad_alt_value);
        return v;
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
		mAltosDroid.unregisterTab(this);
		mAltosDroid = null;
	}

	public void update_ui(AltosState state, AltosGreatCircle from_receiver, Location receiver) {
		mBatteryVoltageView.setText(String.format("%4.2f V", state.battery));
		mBatteryLights.set(state.battery > 3.7);

		mApogeeVoltageView.setText(String.format("%4.2f V", state.drogue_sense));
		mApogeeLights.set(state.drogue_sense > 3.2);

		mMainVoltageView.setText(String.format("%4.2f V", state.main_sense));
		mMainLights.set(state.main_sense > 3.2);

		if (state.data.flight != 0) {
			if (state.data.state <= AltosLib.ao_flight_pad)
				mDataLoggingView.setText("Ready to record");
			else if (state.data.state < AltosLib.ao_flight_landed)
				mDataLoggingView.setText("Recording data");
			else
				mDataLoggingView.setText("Recorded data");
		} else {
			mDataLoggingView.setText("Storage full");
		}
		mDataLoggingLights.set(state.data.flight != 0);

		if (state.gps != null) {
			mGPSLockedView.setText(String.format("%4d sats", state.gps.nsat));
			mGPSLockedLights.set(state.gps.locked && state.gps.nsat >= 4);
			if (state.gps_ready)
				mGPSReadyView.setText("Ready");
			else
				mGPSReadyView.setText(String.format("Waiting %d", state.gps_waiting));
			mGPSReadyLights.set(state.gps_ready);
		}

		if (receiver != null) {
			double altitude = 0;
			if (receiver.hasAltitude())
				altitude = receiver.getAltitude();
			mPadLatitudeView.setText(AltosDroid.pos(receiver.getLatitude(), "N", "S"));
			mPadLongitudeView.setText(AltosDroid.pos(receiver.getLongitude(), "W", "E"));
			mPadAltitudeView.setText(String.format("%4.0f m", altitude));
		}
	}

}

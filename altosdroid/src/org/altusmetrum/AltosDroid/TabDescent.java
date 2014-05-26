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

import org.altusmetrum.altoslib_4.*;

import android.app.Activity;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;
import android.location.Location;

public class TabDescent extends Fragment implements AltosDroidTab {
	AltosDroid mAltosDroid;

	private TextView mSpeedView;
	private TextView mHeightView;
	private TextView mElevationView;
	private TextView mRangeView;
	private TextView mBearingView;
	private TextView mCompassView;
	private TextView mDistanceView;
	private TextView mLatitudeView;
	private TextView mLongitudeView;
	private TextView mApogeeVoltageView;
	private GoNoGoLights mApogeeLights;
	private TextView mMainVoltageView;
	private GoNoGoLights mMainLights;


	@Override
	public void onAttach(Activity activity) {
		super.onAttach(activity);
		mAltosDroid = (AltosDroid) activity;
		mAltosDroid.registerTab(this);
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.tab_descent, container, false);

		mSpeedView     = (TextView) v.findViewById(R.id.speed_value);
		mHeightView    = (TextView) v.findViewById(R.id.height_value);
		mElevationView = (TextView) v.findViewById(R.id.elevation_value);
		mRangeView     = (TextView) v.findViewById(R.id.range_value);
		mBearingView   = (TextView) v.findViewById(R.id.bearing_value);
		mCompassView   = (TextView) v.findViewById(R.id.compass_value);
		mDistanceView  = (TextView) v.findViewById(R.id.distance_value);
		mLatitudeView  = (TextView) v.findViewById(R.id.lat_value);
		mLongitudeView = (TextView) v.findViewById(R.id.lon_value);

		mApogeeVoltageView = (TextView) v.findViewById(R.id.apogee_voltage_value);
		mApogeeLights = new GoNoGoLights((ImageView) v.findViewById(R.id.apogee_redled),
		                                 (ImageView) v.findViewById(R.id.apogee_greenled),
		                                 getResources());

		mMainVoltageView = (TextView) v.findViewById(R.id.main_voltage_value);
		mMainLights = new GoNoGoLights((ImageView) v.findViewById(R.id.main_redled),
		                               (ImageView) v.findViewById(R.id.main_greenled),
		                               getResources());

		return v;
	}


	@Override
	public void onDestroy() {
		super.onDestroy();
		mAltosDroid.unregisterTab(this);
		mAltosDroid = null;
	}

	public void update_ui(AltosState state, AltosGreatCircle from_receiver, Location receiver) {
		if (state != null) {
			mSpeedView.setText(AltosDroid.number("%6.0f m/s", state.speed()));
			mHeightView.setText(AltosDroid.number("%6.0f m", state.height()));
			if (from_receiver != null) {
				mElevationView.setText(AltosDroid.number("%3.0f°", from_receiver.elevation));
				mRangeView.setText(AltosDroid.number("%6.0f m", from_receiver.range));
				mBearingView.setText(AltosDroid.number("%3.0f°", from_receiver.bearing));
				mCompassView.setText(from_receiver.bearing_words(AltosGreatCircle.BEARING_LONG));
				mDistanceView.setText(AltosDroid.number("%6.0f m", from_receiver.distance));
			} else { 
				mElevationView.setText("<unknown>");
				mRangeView.setText("<unknown>");
				mBearingView.setText("<unknown>");
				mCompassView.setText("<unknown>");
				mDistanceView.setText("<unknown>");
			}
			if (state.gps != null) {
				mLatitudeView.setText(AltosDroid.pos(state.gps.lat, "N", "S"));
				mLongitudeView.setText(AltosDroid.pos(state.gps.lon, "W", "E"));
			}

			mApogeeVoltageView.setText(AltosDroid.number("%4.2f V", state.apogee_voltage));
			mApogeeLights.set(state.apogee_voltage > 3.2, state.apogee_voltage == AltosLib.MISSING);

			mMainVoltageView.setText(AltosDroid.number("%4.2f V", state.main_voltage));
			mMainLights.set(state.main_voltage > 3.2, state.main_voltage == AltosLib.MISSING);
		}
	}

}

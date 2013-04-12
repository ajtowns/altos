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
import android.widget.TextView;
import android.location.Location;

public class TabLanded extends Fragment implements AltosDroidTab {
	AltosDroid mAltosDroid;

	private TextView mBearingView;
	private TextView mDistanceView;
	private TextView mLatitudeView;
	private TextView mLongitudeView;
	private TextView mMaxHeightView;
	private TextView mMaxSpeedView;
	private TextView mMaxAccelView;


	@Override
	public void onAttach(Activity activity) {
		super.onAttach(activity);
		mAltosDroid = (AltosDroid) activity;
		mAltosDroid.registerTab(this);
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.tab_landed, container, false);

		mBearingView   = (TextView) v.findViewById(R.id.bearing_value);
		mDistanceView  = (TextView) v.findViewById(R.id.distance_value);
		mLatitudeView  = (TextView) v.findViewById(R.id.lat_value);
		mLongitudeView = (TextView) v.findViewById(R.id.lon_value);
		mMaxHeightView = (TextView) v.findViewById(R.id.max_height_value);
		mMaxSpeedView  = (TextView) v.findViewById(R.id.max_speed_value);
		mMaxAccelView  = (TextView) v.findViewById(R.id.max_accel_value);

		return v;
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
		mAltosDroid.unregisterTab(this);
		mAltosDroid = null;
	}

	public void update_ui(AltosState state, AltosGreatCircle from_receiver, Location receiver) {
		if (from_receiver != null) {
			mBearingView.setText(String.format("%3.0f°", from_receiver.bearing));
			mDistanceView.setText(String.format("%6.0f m", from_receiver.distance));
		}
		if (state.gps != null) {
			mLatitudeView.setText(AltosDroid.pos(state.gps.lat, "N", "S"));
			mLongitudeView.setText(AltosDroid.pos(state.gps.lon, "W", "E"));
		}
		mMaxHeightView.setText(String.format("%6.0f m", state.max_height));
		mMaxAccelView.setText(String.format("%6.0f m/s²", state.max_acceleration));
		mMaxSpeedView.setText(String.format("%6.0f m/s", state.max_speed()));
	}

}

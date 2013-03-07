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

import org.altusmetrum.altoslib_1.AltosState;

import com.google.android.gms.maps.GoogleMap;

import android.app.Activity;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

public class TabMap extends Fragment implements AltosDroidTab {
	AltosDroid mAltosDroid;

	private GoogleMap mMap;


	@Override
	public void onAttach(Activity activity) {
		super.onAttach(activity);
		mAltosDroid = (AltosDroid) activity;
		mAltosDroid.registerTab(this);
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.tab_map, container, false);
		return v;
	}

	@Override
	public void onDestroyView() {
		super.onDestroyView();

		mAltosDroid.unregisterTab(this);
		mAltosDroid = null;

		//Fragment fragment = (getFragmentManager().findFragmentById(R.id.map));
		//FragmentTransaction ft = getActivity().getSupportFragmentManager().beginTransaction();
		//ft.remove(fragment);
		//ft.commit();
	}

	public void update_ui(AltosState state) {
//		mRangeView.setText(String.format("%6.0f m", state.range));
//		if (state.from_pad != null)
//			mBearingView.setText(String.format("%3.0f°", state.from_pad.bearing));
//		mLatitudeView.setText(pos(state.gps.lat, "N", "S"));
//		mLongitudeView.setText(pos(state.gps.lon, "W", "E"));
	}

}

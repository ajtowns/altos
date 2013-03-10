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

import com.google.android.gms.maps.CameraUpdateFactory;
import com.google.android.gms.maps.GoogleMap;
import com.google.android.gms.maps.SupportMapFragment;
import com.google.android.gms.maps.model.BitmapDescriptorFactory;
import com.google.android.gms.maps.model.LatLng;
import com.google.android.gms.maps.model.Marker;
import com.google.android.gms.maps.model.MarkerOptions;

import android.app.Activity;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

public class TabMap extends Fragment implements AltosDroidTab {
	AltosDroid mAltosDroid;

	private SupportMapFragment mMapFragment;
	private GoogleMap mMap;
	private boolean mapLoaded = false;

	private Marker mRocketMarker;
	private Marker mPadMarker;
	private TextView mDistanceView;
	private TextView mBearingView;
	private TextView mLatitudeView;
	private TextView mLongitudeView;

	@Override
	public void onAttach(Activity activity) {
		super.onAttach(activity);
		mAltosDroid = (AltosDroid) activity;
		mAltosDroid.registerTab(this);
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		mMapFragment = new SupportMapFragment() {
			@Override
			public void onActivityCreated(Bundle savedInstanceState) {
				super.onActivityCreated(savedInstanceState);
				setupMap();
			}
		};

	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.tab_map, container, false);
		mDistanceView  = (TextView)v.findViewById(R.id.distance_value);
		mBearingView   = (TextView)v.findViewById(R.id.bearing_value);
		mLatitudeView  = (TextView)v.findViewById(R.id.lat_value);
		mLongitudeView = (TextView)v.findViewById(R.id.lon_value);
		return v;
	}

	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		super.onActivityCreated(savedInstanceState);
		getChildFragmentManager().beginTransaction().add(R.id.map, mMapFragment).commit();
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

	private void setupMap() {
		mMap = mMapFragment.getMap();
		if (mMap != null) {
			mMap.setMyLocationEnabled(true);
			mMap.getUiSettings().setTiltGesturesEnabled(false);
			mMap.getUiSettings().setZoomControlsEnabled(false);
			mMap.moveCamera(CameraUpdateFactory.newLatLngZoom(new LatLng(40.8,-104.7),8));

			mRocketMarker = mMap.addMarker(
					// From: http://mapicons.nicolasmollet.com/markers/industry/military/missile-2/
					new MarkerOptions().icon(BitmapDescriptorFactory.fromResource(R.drawable.rocket))
					                   .position(new LatLng(0,0))
					                   .visible(false)
					);

			mPadMarker = mMap.addMarker(
					new MarkerOptions().icon(BitmapDescriptorFactory.fromResource(R.drawable.pad))
					                   .position(new LatLng(0,0))
					                   .visible(false)
					);

			mapLoaded = true;
		}
	}

	public void update_ui(AltosState state) {
		if (state.from_pad != null) {
			mDistanceView.setText(String.format("%6.0f m", state.from_pad.distance));
			mBearingView.setText(String.format("%3.0f°", state.from_pad.bearing));
		}
		mLatitudeView.setText(AltosDroid.pos(state.gps.lat, "N", "S"));
		mLongitudeView.setText(AltosDroid.pos(state.gps.lon, "W", "E"));

		if (mapLoaded) {
			mRocketMarker.setPosition(new LatLng(state.gps.lat, state.gps.lon));
			mRocketMarker.setVisible(true);

			if (state.state == AltosLib.ao_flight_pad) {
				mPadMarker.setPosition(new LatLng(state.pad_lat, state.pad_lon));
				mPadMarker.setVisible(true);
			}
		}
	}

}

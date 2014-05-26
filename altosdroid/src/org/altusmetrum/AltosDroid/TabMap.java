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

import java.util.Arrays;

import org.altusmetrum.altoslib_4.*;

import com.google.android.gms.maps.CameraUpdateFactory;
import com.google.android.gms.maps.GoogleMap;
import com.google.android.gms.maps.SupportMapFragment;
import com.google.android.gms.maps.model.BitmapDescriptorFactory;
import com.google.android.gms.maps.model.LatLng;
import com.google.android.gms.maps.model.Marker;
import com.google.android.gms.maps.model.MarkerOptions;
import com.google.android.gms.maps.model.Polyline;
import com.google.android.gms.maps.model.PolylineOptions;

import android.app.Activity;
import android.graphics.Color;
import android.os.Bundle;
import android.support.v4.app.Fragment;
//import android.support.v4.app.FragmentTransaction;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.location.Location;

public class TabMap extends Fragment implements AltosDroidTab {
	AltosDroid mAltosDroid;

	private SupportMapFragment mMapFragment;
	private GoogleMap mMap;
	private boolean mapLoaded = false;

	private Marker mRocketMarker;
	private Marker mPadMarker;
	private Polyline mPolyline;

	private TextView mDistanceView;
	private TextView mBearingView;
	private TextView mTargetLatitudeView;
	private TextView mTargetLongitudeView;
	private TextView mReceiverLatitudeView;
	private TextView mReceiverLongitudeView;

	private double mapAccuracy = -1;

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
		mTargetLatitudeView  = (TextView)v.findViewById(R.id.target_lat_value);
		mTargetLongitudeView = (TextView)v.findViewById(R.id.target_lon_value);
		mReceiverLatitudeView  = (TextView)v.findViewById(R.id.receiver_lat_value);
		mReceiverLongitudeView = (TextView)v.findViewById(R.id.receiver_lon_value);
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

			mPolyline = mMap.addPolyline(
					new PolylineOptions().add(new LatLng(0,0), new LatLng(0,0))
					                     .width(3)
					                     .color(Color.BLUE)
					                     .visible(false)
					);

			mapLoaded = true;
		}
	}

	private void center(double lat, double lon, double accuracy) {
		if (mapAccuracy < 0 || accuracy < mapAccuracy/10) {
			mMap.moveCamera(CameraUpdateFactory.newLatLngZoom(new LatLng(lat, lon),14));
			mapAccuracy = accuracy;
		}
	}

	public void update_ui(AltosState state, AltosGreatCircle from_receiver, Location receiver) {
		if (from_receiver != null) {
			mBearingView.setText(String.format("%3.0f°", from_receiver.bearing));
			mDistanceView.setText(String.format("%6.0f m", from_receiver.distance));
		}

		if (state != null) {
			if (mapLoaded) {
				if (state.gps != null) {
					mRocketMarker.setPosition(new LatLng(state.gps.lat, state.gps.lon));
					mRocketMarker.setVisible(true);

					mPolyline.setPoints(Arrays.asList(new LatLng(state.pad_lat, state.pad_lon), new LatLng(state.gps.lat, state.gps.lon)));
					mPolyline.setVisible(true);
				}

				if (state.state == AltosLib.ao_flight_pad) {
					mPadMarker.setPosition(new LatLng(state.pad_lat, state.pad_lon));
					mPadMarker.setVisible(true);
				}
			}
			if (state.gps != null) {
				mTargetLatitudeView.setText(AltosDroid.pos(state.gps.lat, "N", "S"));
				mTargetLongitudeView.setText(AltosDroid.pos(state.gps.lon, "W", "E"));
				if (state.gps.locked && state.gps.nsat >= 4)
					center (state.gps.lat, state.gps.lon, 10);
			}
		}

		if (receiver != null) {
			double accuracy;

			if (receiver.hasAccuracy())
				accuracy = receiver.getAccuracy();
			else
				accuracy = 1000;
			mReceiverLatitudeView.setText(AltosDroid.pos(receiver.getLatitude(), "N", "S"));
			mReceiverLongitudeView.setText(AltosDroid.pos(receiver.getLongitude(), "W", "E"));
			center (receiver.getLatitude(), receiver.getLongitude(), accuracy);
		}

	}

}

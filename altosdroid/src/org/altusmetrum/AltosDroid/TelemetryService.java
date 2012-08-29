/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.altusmetrum.AltosDroid;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;
import android.widget.Toast;

// Need the following import to get access to the app resources, since this
// class is in a sub-package.
import org.altusmetrum.AltosDroid.R;



public class TelemetryService extends Service {
    private NotificationManager mNM;

    // Unique Identification Number for the Notification.
    // We use it on Notification start, and to cancel it.
    private int NOTIFICATION = R.string.telemetry_service_label;

    /**
     * Class for clients to access.  Because we know this service always
     * runs in the same process as its clients, we don't need to deal with
     * IPC.
     */
    public class TelemetryBinder extends Binder {
        TelemetryService getService() {
            return TelemetryService.this;
        }
    }

    @Override
    public void onCreate() {
        // Create a reference to the NotificationManager so that we can update our notifcation text later
        mNM = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i("TelemetryService", "Received start id " + startId + ": " + intent);

        CharSequence text = getText(R.string.telemetry_service_started);

        // Create notification to be displayed while the service runs
        Notification notification = new Notification(R.drawable.am_status_c, text, 0);

        // The PendingIntent to launch our activity if the user selects this notification
        PendingIntent contentIntent = PendingIntent.getActivity(this, 0,
                new Intent(this, TelemetryServiceActivities.Controller.class), 0);

        // Set the info for the views that show in the notification panel.
        notification.setLatestEventInfo(this, getText(R.string.telemetry_service_label), text, contentIntent);

        // Set the notification to be in the "Ongoing" section.
        notification.flags |= Notification.FLAG_ONGOING_EVENT;

        // Move us into the foreground.
        startForeground(NOTIFICATION, notification);

        // We want this service to continue running until it is explicitly
        // stopped, so return sticky.
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        // Demote us from the foreground, and cancel the persistent notification.
        stopForeground(true);

        // Tell the user we stopped.
        Toast.makeText(this, R.string.telemetry_service_stopped, Toast.LENGTH_SHORT).show();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    // This is the object that receives interactions from clients.  See
    // RemoteService for a more complete example.
    private final IBinder mBinder = new TelemetryBinder();

}

/*
 * Copyright © 2014 Keith Packard <keithp@keithp.com>
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

package org.altusmetrum.altosuilib_2;

import java.io.*;
import java.net.*;
import java.util.*;

public class AltosUIMapStore {
	String					url;
	File					file;
	LinkedList<AltosUIMapStoreListener>	listeners = new LinkedList<AltosUIMapStoreListener>();

	static final int			success = 0;
	static final int			loading = 1;
	static final int			failed = 2;
	static final int			bad_request = 3;
	static final int			forbidden = 4;

	int					status;

	public int status() {
		return status;
	}

	public synchronized void add_listener(AltosUIMapStoreListener listener) {
		if (!listeners.contains(listener))
			listeners.add(listener);
	}

	public synchronized void remove_listener(AltosUIMapStoreListener listener) {
		listeners.remove(listener);
	}

	private synchronized void notify_listeners(int status) {
		this.status = status;
		for (AltosUIMapStoreListener listener : listeners)
			listener.notify_store(this, status);
	}

	static Object	forbidden_lock = new Object();
	static long	forbidden_time;
	static boolean	forbidden_set;

	private int fetch_url() {
		URL u;

		try {
			u = new URL(url);
		} catch (java.net.MalformedURLException e) {
			return bad_request;
		}

		byte[] data;
		URLConnection uc = null;
		try {
			uc = u.openConnection();
			String type = uc.getContentType();
			int contentLength = uc.getContentLength();
			if (uc instanceof HttpURLConnection) {
				int response = ((HttpURLConnection) uc).getResponseCode();
				switch (response) {
				case HttpURLConnection.HTTP_FORBIDDEN:
				case HttpURLConnection.HTTP_PAYMENT_REQUIRED:
				case HttpURLConnection.HTTP_UNAUTHORIZED:
					synchronized (forbidden_lock) {
						forbidden_time = System.nanoTime();
						forbidden_set = true;
						return forbidden;
					}
				}
			}
			InputStream in = new BufferedInputStream(uc.getInputStream());
			int bytesRead = 0;
			int offset = 0;
			data = new byte[contentLength];
			while (offset < contentLength) {
				bytesRead = in.read(data, offset, data.length - offset);
				if (bytesRead == -1)
					break;
				offset += bytesRead;
			}
			in.close();

			if (offset != contentLength)
				return failed;

		} catch (IOException e) {
			return failed;
		}

		try {
			FileOutputStream out = new FileOutputStream(file);
			out.write(data);
			out.flush();
			out.close();
		} catch (FileNotFoundException e) {
			return bad_request;
		} catch (IOException e) {
			if (file.exists())
				file.delete();
			return bad_request;
		}
		return success;
	}

	static Object	fetch_lock = new Object();

	static final long	forbidden_interval = 60l * 1000l * 1000l * 1000l;
	static final long 	google_maps_ratelimit_ms = 1200;

	class loader implements Runnable {

		public void run() {
			if (file.exists()) {
				notify_listeners(success);
				return;
			}

			synchronized(forbidden_lock) {
				if (forbidden_set && (System.nanoTime() - forbidden_time) < forbidden_interval) {
					notify_listeners(forbidden);
					return;
				}
			}

			int new_status;

			if (!AltosUIVersion.has_google_maps_api_key()) {
				synchronized (fetch_lock) {
					long startTime = System.nanoTime();
					new_status = fetch_url();
					if (new_status == success) {
						long duration_ms = (System.nanoTime() - startTime) / 1000000;
						if (duration_ms < google_maps_ratelimit_ms) {
							try {
								Thread.sleep(google_maps_ratelimit_ms - duration_ms);
							} catch (InterruptedException e) {
								Thread.currentThread().interrupt();
							}
						}
					}
				}
			} else {
				new_status = fetch_url();
			}
			notify_listeners(new_status);
		}
	}

	private void load() {
		loader	l = new loader();
		Thread	lt = new Thread(l);
		lt.start();
	}

	private AltosUIMapStore (String url, File file) {
		this.url = url;
		this.file = file;

		if (file.exists())
			status = success;
		else {
			status = loading;
			load();
		}
	}

	public boolean equals(AltosUIMapStore other) {
		return url.equals(other.url);
	}

	static HashMap<String,AltosUIMapStore> stores = new HashMap<String,AltosUIMapStore>();

	public static AltosUIMapStore get(String url, File file) {
		AltosUIMapStore	store;
		synchronized(stores) {
			if (stores.containsKey(url)) {
				store = stores.get(url);
			} else {
				store = new AltosUIMapStore(url, file);
				stores.put(url, store);
			}
		}
		return store;
	}

}

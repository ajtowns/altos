/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
 * Copyright © 2012 Mike Beattie <mike@ethernal.org>
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

import android.speech.tts.TextToSpeech;
import android.speech.tts.TextToSpeech.OnInitListener;

import org.altusmetrum.altoslib_4.*;

public class AltosVoice {

	private TextToSpeech tts         = null;
	private boolean      tts_enabled = false;

	private IdleThread   idle_thread = null;

	private AltosState   old_state   = null;

	public AltosVoice(AltosDroid a) {

		tts = new TextToSpeech(a, new OnInitListener() {
			public void onInit(int status) {
				if (status == TextToSpeech.SUCCESS) tts_enabled = true;
				if (tts_enabled) {
					idle_thread = new IdleThread();
				}
			}
		});

	}

	public void speak(String s) {
		if (!tts_enabled) return;
		tts.speak(s, TextToSpeech.QUEUE_ADD, null);
	}

	public void stop() {
		if (tts != null) tts.shutdown();
		if (idle_thread != null) {
			idle_thread.interrupt();
			idle_thread = null;
		}
	}

	public void tell(AltosState state) {
		if (!tts_enabled) return;

		boolean	spoke = false;
		if (old_state == null || old_state.state != state.state) {
			if (state.state != AltosLib.ao_flight_stateless)
				speak(state.state_name());
			if ((old_state == null || old_state.state <= AltosLib.ao_flight_boost) &&
			    state.state > AltosLib.ao_flight_boost) {
				if (state.max_speed() != AltosLib.MISSING)
					speak(String.format("max speed: %d meters per second.", (int) (state.max_speed() + 0.5)));
				spoke = true;
			} else if ((old_state == null || old_state.state < AltosLib.ao_flight_drogue) &&
			           state.state >= AltosLib.ao_flight_drogue) {
				if (state.max_height() != AltosLib.MISSING)
					speak(String.format("max height: %d meters.", (int) (state.max_height() + 0.5)));
				spoke = true;
			}
		}
		if (old_state == null || old_state.gps_ready != state.gps_ready) {
			if (state.gps_ready) {
				speak("GPS ready");
				spoke = true;
			} else if (old_state != null) {
				speak("GPS lost");
				spoke = true;
			}
		}
		old_state = state;
		idle_thread.notice(state, spoke);
	}


	class IdleThread extends Thread {
		boolean	           started;
		private AltosState state;
		int                reported_landing;
		int                report_interval;
		long               report_time;

		public synchronized void report(boolean last) {
			if (state == null)
				return;

			/* reset the landing count once we hear about a new flight */
			if (state.state < AltosLib.ao_flight_drogue)
				reported_landing = 0;

			/* Shut up once the rocket is on the ground */
			if (reported_landing > 2) {
				return;
			}

			/* If the rocket isn't on the pad, then report height */
			if (((AltosLib.ao_flight_drogue <= state.state &&
			      state.state < AltosLib.ao_flight_landed) ||
			     state.state == AltosLib.ao_flight_stateless) &&
			    state.range >= 0)
			{
				speak(String.format("Height %d, bearing %s %d, elevation %d, range %d.\n",
				                    (int) (state.height() + 0.5),
	                                state.from_pad.bearing_words(
	                                      AltosGreatCircle.BEARING_VOICE),
				                    (int) (state.from_pad.bearing + 0.5),
				                    (int) (state.elevation + 0.5),
				                    (int) (state.range + 0.5)));
			} else if (state.state > AltosLib.ao_flight_pad) {
				if (state.height() != AltosLib.MISSING)
					speak(String.format("%d meters", (int) (state.height() + 0.5)));
			} else {
				reported_landing = 0;
			}

			/* If the rocket is coming down, check to see if it has landed;
			 * either we've got a landed report or we haven't heard from it in
			 * a long time
			 */
			if (state.state >= AltosLib.ao_flight_drogue &&
			    (last ||
			     System.currentTimeMillis() - state.received_time >= 15000 ||
			     state.state == AltosLib.ao_flight_landed))
			{
				if (Math.abs(state.speed()) < 20 && state.height() < 100)
					speak("rocket landed safely");
				else
					speak("rocket may have crashed");
				if (state.from_pad != null)
					speak(String.format("Bearing %d degrees, range %d meters.",
					                    (int) (state.from_pad.bearing + 0.5),
					                    (int) (state.from_pad.distance + 0.5)));
				++reported_landing;
			}
		}

		long now () {
			return System.currentTimeMillis();
		}

		void set_report_time() {
			report_time = now() + report_interval;
		}

		public void run () {
			try {
				for (;;) {
					set_report_time();
					for (;;) {
						synchronized (this) {
							long sleep_time = report_time - now();
							if (sleep_time <= 0)
								break;
							wait(sleep_time);
						}
					}
					report(false);
				}
			} catch (InterruptedException ie) {
			}
		}

		public synchronized void notice(AltosState new_state, boolean spoken) {
			AltosState old_state = state;
			state = new_state;
			if (!started && state.state > AltosLib.ao_flight_pad) {
				started = true;
				start();
			}

			if (state.state < AltosLib.ao_flight_drogue)
				report_interval = 10000;
			else
				report_interval = 20000;
			if (old_state != null && old_state.state != state.state) {
				report_time = now();
				this.notify();
			} else if (spoken)
				set_report_time();
		}

		public IdleThread() {
			state = null;
			reported_landing = 0;
			report_interval = 10000;
		}
	}

}

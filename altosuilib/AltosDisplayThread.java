/*
 * Copyright © 2010 Keith Packard <keithp@keithp.com>
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

import java.awt.*;
import javax.swing.*;
import java.io.*;
import java.text.*;
import org.altusmetrum.altoslib_4.*;

public class AltosDisplayThread extends Thread {

	Frame			parent;
	IdleThread		idle_thread;
	AltosVoice		voice;
	AltosFlightReader	reader;
	AltosState		old_state, state;
	AltosListenerState	listener_state;
	AltosFlightDisplay	display;

	synchronized void show_safely() {
		final AltosState my_state = state;
		final AltosListenerState my_listener_state = listener_state;
		Runnable r = new Runnable() {
				public void run() {
					try {
						display.show(my_state, my_listener_state);
					} catch (Exception ex) {
					}
				}
			};
		SwingUtilities.invokeLater(r);
	}

	void reading_error_internal() {
		JOptionPane.showMessageDialog(parent,
					      String.format("Error reading from \"%s\"", reader.name),
					      "Telemetry Read Error",
					      JOptionPane.ERROR_MESSAGE);
	}

	void reading_error_safely() {
		Runnable r = new Runnable() {
				public void run() {
					try {
						reading_error_internal();
					} catch (Exception ex) {
					}
				}
			};
		SwingUtilities.invokeLater(r);
	}

	class IdleThread extends Thread {

		boolean	started;
		int	reported_landing;
		int	report_interval;
		long	report_time;

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
			if (AltosLib.ao_flight_drogue <= state.state &&
			    state.state < AltosLib.ao_flight_landed &&
			    state.from_pad != null &&
			    state.range >= 0)
			{
				voice.speak("Height %s, bearing %s %d, elevation %d, range %s.\n",
					    AltosConvert.height.say(state.height()),
					    state.from_pad.bearing_words(
						    AltosGreatCircle.BEARING_VOICE),
					    (int) (state.from_pad.bearing + 0.5),
					    (int) (state.elevation + 0.5),
					    AltosConvert.distance.say(state.range));
			} else if (state.state > AltosLib.ao_flight_pad && state.height() != AltosLib.MISSING) {
				voice.speak(AltosConvert.height.say_units(state.height()));
			} else {
				reported_landing = 0;
			}

			/* If the rocket is coming down, check to see if it has landed;
			 * either we've got a landed report or we haven't heard from it in
			 * a long time
			 */
			if (state.state != AltosLib.ao_flight_stateless &&
			    state.state >= AltosLib.ao_flight_drogue &&
			    (last ||
			     System.currentTimeMillis() - state.received_time >= 15000 ||
			     state.state == AltosLib.ao_flight_landed))
			{
				if (Math.abs(state.speed()) < 20 && state.height() < 100)
					voice.speak("rocket landed safely");
				else
					voice.speak("rocket may have crashed");
				if (state.from_pad != null)
					voice.speak("Bearing %d degrees, range %s.",
						    (int) (state.from_pad.bearing + 0.5),
						    AltosConvert.distance.say_units(state.from_pad.distance));
				++reported_landing;
				if (state.state != AltosLib.ao_flight_landed) {
					state.state = AltosLib.ao_flight_landed;
					show_safely();
				}
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
					if (reader.has_monitor_battery()) {
						listener_state.battery = reader.monitor_battery();
						show_safely();
					}
					set_report_time();
					for (;;) {
						voice.drain();
						synchronized (this) {
							long	sleep_time = report_time - now();
							if (sleep_time <= 0)
								break;
							wait(sleep_time);
						}
					}

					report(false);
				}
			} catch (InterruptedException ie) {
				try {
					voice.drain();
				} catch (InterruptedException iie) { }
			}
		}

		public synchronized void notice(boolean spoken) {
			if (old_state != null && old_state.state != state.state) {
				report_time = now();
				this.notify();
			} else if (spoken)
				set_report_time();
		}

		public IdleThread() {
			reported_landing = 0;
			report_interval = 10000;
		}
	}

	synchronized boolean tell() {
		boolean	ret = false;
		if (old_state == null || old_state.state != state.state) {
			if (state.state != AltosLib.ao_flight_stateless)
				voice.speak(state.state_name());
			if ((old_state == null || old_state.state <= AltosLib.ao_flight_boost) &&
			    state.state > AltosLib.ao_flight_boost) {
				if (state.max_speed() != AltosLib.MISSING)
					voice.speak("max speed: %s.",
						    AltosConvert.speed.say_units(state.max_speed() + 0.5));
				ret = true;
			} else if ((old_state == null || old_state.state < AltosLib.ao_flight_drogue) &&
				   state.state >= AltosLib.ao_flight_drogue) {
				if (state.max_height() != AltosLib.MISSING)
					voice.speak("max height: %s.",
						    AltosConvert.height.say_units(state.max_height() + 0.5));
				ret = true;
			}
		}
		if (old_state == null || old_state.gps_ready != state.gps_ready) {
			if (state.gps_ready) {
				voice.speak("GPS ready");
				ret = true;
			}
			else if (old_state != null) {
				voice.speak("GPS lost");
				ret = true;
			}
		}
		old_state = state;
		return ret;
	}

	public void run() {
		boolean		interrupted = false;
		boolean		told;

		idle_thread = new IdleThread();
		idle_thread.start();

		try {
			for (;;) {
				try {
					state = reader.read();
					if (state == null)
						break;
					reader.update(state);
					show_safely();
					told = tell();
					idle_thread.notice(told);
				} catch (ParseException pp) {
					System.out.printf("Parse error: %d \"%s\"\n", pp.getErrorOffset(), pp.getMessage());
				} catch (AltosCRCException ce) {
					++listener_state.crc_errors;
					show_safely();
				}
			}
		} catch (InterruptedException ee) {
			interrupted = true;
		} catch (IOException ie) {
			reading_error_safely();
		} finally {
			if (!interrupted)
				idle_thread.report(true);
			reader.close(interrupted);
			idle_thread.interrupt();
			try {
				idle_thread.join();
			} catch (InterruptedException ie) {}
		}
	}

	public AltosDisplayThread(Frame in_parent, AltosVoice in_voice, AltosFlightDisplay in_display, AltosFlightReader in_reader) {
		listener_state = new AltosListenerState();
		parent = in_parent;
		voice = in_voice;
		display = in_display;
		reader = in_reader;
		display.reset();
	}
}

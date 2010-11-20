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

package altosui;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.table.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.util.concurrent.LinkedBlockingQueue;

public class AltosDisplayThread extends Thread {

	Frame			parent;
	IdleThread		idle_thread;
	AltosVoice		voice;
	String			name;
	AltosFlightReader	reader;
	int			crc_errors;
	AltosFlightDisplay	display;

	synchronized void show(AltosState state, int crc_errors) {
		if (state != null)
			display.show(state, crc_errors);
	}

	class IdleThread extends Thread {

		boolean	started;
		private AltosState state;
		int	reported_landing;
		int	report_interval;
		long	report_time;

		public synchronized void report(boolean last) {
			if (state == null)
				return;

			/* reset the landing count once we hear about a new flight */
			if (state.state < Altos.ao_flight_drogue)
				reported_landing = 0;

			/* Shut up once the rocket is on the ground */
			if (reported_landing > 2) {
				return;
			}

			/* If the rocket isn't on the pad, then report height */
			if (Altos.ao_flight_drogue <= state.state &&
			    state.state < Altos.ao_flight_landed &&
			    state.range >= 0)
			{
				voice.speak("Height %d, bearing %s %d, elevation %d, range %d.\n",
					    (int) (state.height + 0.5),
                        state.from_pad.bearing_words(
                            AltosGreatCircle.BEARING_VOICE),
					    (int) (state.from_pad.bearing + 0.5),
					    (int) (state.elevation + 0.5),
					    (int) (state.range + 0.5));
			} else if (state.state > Altos.ao_flight_pad) {
				voice.speak("%d meters", (int) (state.height + 0.5));
			} else {
				reported_landing = 0;
			}

			/* If the rocket is coming down, check to see if it has landed;
			 * either we've got a landed report or we haven't heard from it in
			 * a long time
			 */
			if (state.state >= Altos.ao_flight_drogue &&
			    (last ||
			     System.currentTimeMillis() - state.report_time >= 15000 ||
			     state.state == Altos.ao_flight_landed))
			{
				if (Math.abs(state.baro_speed) < 20 && state.height < 100)
					voice.speak("rocket landed safely");
				else
					voice.speak("rocket may have crashed");
				if (state.from_pad != null)
					voice.speak("Bearing %d degrees, range %d meters.",
						    (int) (state.from_pad.bearing + 0.5),
						    (int) (state.from_pad.distance + 0.5));
				++reported_landing;
				if (state.state != Altos.ao_flight_landed) {
					state.state = Altos.ao_flight_landed;
					show(state, 0);
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

		public synchronized void notice(AltosState new_state, boolean spoken) {
			AltosState old_state = state;
			state = new_state;
			if (!started && state.state > Altos.ao_flight_pad) {
				started = true;
				start();
			}

			if (state.state < Altos.ao_flight_drogue)
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

	boolean tell(AltosState state, AltosState old_state) {
		boolean	ret = false;
		if (old_state == null || old_state.state != state.state) {
			voice.speak(state.data.state());
			if ((old_state == null || old_state.state <= Altos.ao_flight_boost) &&
			    state.state > Altos.ao_flight_boost) {
				voice.speak("max speed: %d meters per second.",
					    (int) (state.max_speed + 0.5));
				ret = true;
			} else if ((old_state == null || old_state.state < Altos.ao_flight_drogue) &&
				   state.state >= Altos.ao_flight_drogue) {
				voice.speak("max height: %d meters.",
					    (int) (state.max_height + 0.5));
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
		String		line;
		AltosState	state = null;
		AltosState	old_state = null;
		boolean		told;

		idle_thread = new IdleThread();

		display.reset();
		try {
			for (;;) {
				try {
					AltosRecord record = reader.read();
					if (record == null)
						break;
					old_state = state;
					state = new AltosState(record, state);
					reader.update(state);
					show(state, crc_errors);
					told = tell(state, old_state);
					idle_thread.notice(state, told);
				} catch (ParseException pp) {
					System.out.printf("Parse error: %d \"%s\"\n", pp.getErrorOffset(), pp.getMessage());
				} catch (AltosCRCException ce) {
					++crc_errors;
					show(state, crc_errors);
				}
			}
		} catch (InterruptedException ee) {
			interrupted = true;
		} catch (IOException ie) {
			JOptionPane.showMessageDialog(parent,
						      String.format("Error reading from \"%s\"", name),
						      "Telemetry Read Error",
						      JOptionPane.ERROR_MESSAGE);
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
		parent = in_parent;
		voice = in_voice;
		display = in_display;
		reader = in_reader;
	}
}

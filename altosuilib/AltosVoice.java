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

import com.sun.speech.freetts.Voice;
import com.sun.speech.freetts.VoiceManager;
import java.util.concurrent.LinkedBlockingQueue;

public class AltosVoice implements Runnable {
	VoiceManager			voice_manager;
	Voice				voice;
	LinkedBlockingQueue<String>	phrases;
	Thread				thread;
	boolean				busy;

	final static String voice_name = "kevin16";

	public void run() {
		try {
			for (;;) {
				String s = phrases.take();
				voice.speak(s);
				synchronized(this) {
					if (phrases.isEmpty()) {
						busy = false;
						notifyAll();
					}
				}
			}
		} catch (InterruptedException e) {
		}
	}

	public synchronized void drain() throws InterruptedException {
		while (busy)
			wait();
	}

	public void speak_always(String s) {
		try {
			if (voice != null) {
				synchronized(this) {
					busy = true;
					phrases.put(s);
				}
			}
		} catch (InterruptedException e) {
		}
	}

	public void speak(String s) {
		if (AltosUIPreferences.voice())
			speak_always(s);
	}

	public void speak(String format, Object... parameters) {
		speak(String.format(format, parameters));
	}

	public AltosVoice () {
		busy = false;
		voice_manager = VoiceManager.getInstance();
		voice = voice_manager.getVoice(voice_name);
		if (voice != null) {
			voice.allocate();
			phrases = new LinkedBlockingQueue<String> ();
			thread = new Thread(this);
			thread.start();
		} else {
			System.out.printf("Voice manager failed to open %s\n", voice_name);
			Voice[] voices = voice_manager.getVoices();
			System.out.printf("Available voices:\n");
			for (int i = 0; i < voices.length; i++) {
				System.out.println("    " + voices[i].getName()
						   + " (" + voices[i].getDomain() + " domain)");
			}
		}
	}
}

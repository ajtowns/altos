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

import java.io.*;
import java.lang.*;
import java.util.*;
import java.text.ParseException;
import java.util.concurrent.LinkedBlockingQueue;
import altosui.AltosSerial;
import altosui.AltosFile;

/*
 * This creates a thread to capture telemetry data and write it to
 * a log file
 */
class AltosLog implements Runnable {

	LinkedBlockingQueue<String>	input_queue;
	LinkedBlockingQueue<String>	pending_queue;
	int				serial;
	int				flight;
	FileWriter			log_file;
	Thread				log_thread;

	void close() throws IOException {
		if (log_file != null)
			log_file.close();
	}

	boolean open (AltosTelemetry telem) throws IOException {
		AltosFile	a = new AltosFile(telem);

		log_file = new FileWriter(a, true);
		if (log_file != null) {
			while (!pending_queue.isEmpty()) {
				try {
					String s = pending_queue.take();
					log_file.write(s);
					log_file.write('\n');
				} catch (InterruptedException ie) {
				}
			}
			log_file.flush();
		}
		return log_file != null;
	}

	public void run () {
		try {
			for (;;) {
				String	line = input_queue.take();
				try {
					AltosTelemetry	telem = new AltosTelemetry(line);
					if (telem.serial != serial || telem.flight != flight || log_file == null) {
						close();
						serial = telem.serial;
						flight = telem.flight;
						open(telem);
					}
				} catch (ParseException pe) {
				}
				if (log_file != null) {
					log_file.write(line);
					log_file.write('\n');
					log_file.flush();
				} else
					pending_queue.put(line);
			}
		} catch (InterruptedException ie) {
		} catch (IOException ie) {
		}
		try {
			close();
		} catch (IOException ie) {
		}
	}

	public AltosLog (AltosSerial s) {
		pending_queue = new LinkedBlockingQueue<String> ();
		input_queue = new LinkedBlockingQueue<String> ();
		s.add_monitor(input_queue);
		serial = -1;
		flight = -1;
		log_file = null;
		log_thread = new Thread(this);
		log_thread.start();
	}
}

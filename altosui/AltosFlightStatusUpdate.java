/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

import java.awt.event.*;
import org.altusmetrum.altoslib_2.*;

public class AltosFlightStatusUpdate implements ActionListener {

	public AltosState		saved_state;
	public AltosListenerState	saved_listener_state;
	AltosFlightStatus		flightStatus;

	public void actionPerformed (ActionEvent e) {
		if (saved_state != null) {
			if (saved_listener_state == null)
				saved_listener_state = new AltosListenerState();
			flightStatus.show(saved_state, saved_listener_state);
		}
	}

	public AltosFlightStatusUpdate (AltosFlightStatus in_flightStatus) {
		flightStatus = in_flightStatus;
	}
}


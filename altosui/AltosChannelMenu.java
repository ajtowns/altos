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

import java.awt.event.*;
import javax.swing.*;

public class AltosChannelMenu extends JComboBox<String> implements ActionListener {
	int				channel;

	public AltosChannelMenu(int current_channel) {

		channel = current_channel;

		for (int c = 0; c <= 9; c++)
			addItem(String.format("Channel %1d (%7.3fMHz)", c, 434.550 + c * 0.1));
		setSelectedIndex(channel);
		setMaximumRowCount(10);
	}

}

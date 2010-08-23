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

public class AltosChannelMenu extends JMenu implements ActionListener {
	ButtonGroup			group;
	int				channel;
	LinkedList<ActionListener>	listeners;

	public void addActionListener(ActionListener l) {
		listeners.add(l);
	}

	public void actionPerformed(ActionEvent e) {
		channel = Integer.parseInt(e.getActionCommand());

		ListIterator<ActionListener>	i = listeners.listIterator();

		ActionEvent newe = new ActionEvent(this, channel, e.getActionCommand());
		while (i.hasNext()) {
			ActionListener	listener = i.next();
			listener.actionPerformed(newe);
		}
	}

	public AltosChannelMenu(int current_channel) {
		super("Channel", true);
		group = new ButtonGroup();

		channel = current_channel;

		listeners = new LinkedList<ActionListener>();
		for (int c = 0; c <= 9; c++) {
			JRadioButtonMenuItem radioitem = new JRadioButtonMenuItem(String.format("Channel %1d (%7.3fMHz)", c,
												434.550 + c * 0.1),
							     c == channel);
			radioitem.setActionCommand(String.format("%d", c));
			radioitem.addActionListener(this);
			add(radioitem);
			group.add(radioitem);
		}
	}

}

/*
 * Copyright © 2010 Anthony Towns <aj@erisian.com.au>
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
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.MouseInputAdapter;

class GrabNDrag extends MouseInputAdapter {
	private JComponent scroll;
	private Point startPt = new Point();

	public GrabNDrag(JComponent scroll) {
		this.scroll = scroll;
		scroll.addMouseMotionListener(this);
		scroll.addMouseListener(this);
		scroll.setAutoscrolls(true);
	}

	public static boolean grab_n_drag(MouseEvent e) {
		return e.getModifiers() == InputEvent.BUTTON1_MASK;
	}

	public void mousePressed(MouseEvent e) {
		if (grab_n_drag(e))
			startPt.setLocation(e.getPoint());
	}
	public void mouseDragged(MouseEvent e) {
		if (grab_n_drag(e)) {
			int xd = e.getX() - startPt.x;
			int yd = e.getY() - startPt.y;

			Rectangle r = scroll.getVisibleRect();
			r.x -= xd;
			r.y -= yd;
			scroll.scrollRectToVisible(r);
		}
	}
}

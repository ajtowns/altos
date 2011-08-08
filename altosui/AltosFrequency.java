/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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
import javax.swing.event.*;
import javax.swing.plaf.basic.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.util.concurrent.*;

public class AltosFrequency {
	double	frequency;
	String	description;

	public String toString() {
		return String.format("%7.3f MHz %-20.20s",
				     frequency, description);
	}

	public String toShortString() {
		return String.format("%7.3f MHz %s",
				     frequency, description);
	}

	public boolean close(double f) {
		double	diff = Math.abs(frequency - f);

		return diff < 0.010;
	}

	public AltosFrequency(double f, String d) {
		frequency = f;
		description = d;
	}
}
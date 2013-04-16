/*
 * Copyright © 2013 Mike Beattie <mike@ethernal.org>
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

import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.widget.ImageView;

public class GoNoGoLights {
	private Boolean state;
	private Boolean missing;
	private Boolean set;

	private ImageView red;
	private ImageView green;

	private Drawable dRed;
	private Drawable dGreen;
	private Drawable dGray;

	public GoNoGoLights(ImageView in_red, ImageView in_green, Resources r) {
		red = in_red;
		green = in_green;
		state = false;
		missing = true;
		set = false;

		dRed   = r.getDrawable(R.drawable.redled);
		dGreen = r.getDrawable(R.drawable.greenled);
		dGray  = r.getDrawable(R.drawable.grayled);
	}

	public void set(Boolean s, Boolean m) {
		if (set && s == state && m == missing) return;
		state = s;
		missing = m;
		set = true;
		if (missing) {
			red.setImageDrawable(dGray);
			green.setImageDrawable(dGray);
		} else if (state) {
			red.setImageDrawable(dGray);
			green.setImageDrawable(dGreen);
		} else {
			red.setImageDrawable(dRed);
			green.setImageDrawable(dGray);
		}
	}
}

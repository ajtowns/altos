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

#include "ao.h"

static void
ao_terraui(void)
{
	for (;;) {
		char	b = ao_button_get();
		switch (b) {
		case 1:
			ao_beep_for(AO_BEEP_LOW, AO_MS_TO_TICKS(200));
			break;
		case 2:
			ao_beep_for(AO_BEEP_MID, AO_MS_TO_TICKS(200));
			break;
		case 3:
			ao_beep_for(AO_BEEP_HIGH, AO_MS_TO_TICKS(200));
			break;
		}
	}
}

__xdata static struct ao_task ao_terraui_task;

void
ao_terraui_init(void)
{
	ao_add_task(&ao_terraui_task, ao_terraui, "ui");
}

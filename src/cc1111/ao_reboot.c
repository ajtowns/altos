/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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

/* Use the watchdog timer to force a complete reboot
 */
void
ao_reboot(void)
{
	WDCTL = WDCTL_EN | WDCTL_MODE_WATCHDOG | WDCTL_INT_32768;
	ao_delay(AO_SEC_TO_TICKS(2));
	ao_panic(AO_PANIC_REBOOT);
}

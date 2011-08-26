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

/*
 * CC1111 definitions and code fragments for AltOS
 */

#ifndef _AO_ARCH_H_
#define _AO_ARCH_H_

#include "cc1111.h"

#define ao_arch_reboot() do {					\
	WDCTL = WDCTL_EN | WDCTL_MODE_WATCHDOG | WDCTL_INT_64;	\
	ao_delay(AO_SEC_TO_TICKS(2));				\
	} while (0)
	
#endif /* _AO_ARCH_H_ */

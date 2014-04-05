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

#ifndef _AO_BTM_H_
#define _AO_BTM_H_

/* ao_btm.c */

/* If bt_link is on P2, this interrupt is shared by USB, so the USB
 * code calls this function. Otherwise, it's a regular ISR.
 */

void
ao_btm_isr(void)
#if BT_LINK_ON_P1
	__interrupt 15
#endif
	;
void
ao_btm_init(void);

#endif /* _AO_BTM_H_ */

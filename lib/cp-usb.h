/*
 * Copyright © 2008 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#ifndef _CP_USB_H_
#define _CP_USB_H_
#include <usb.h>

struct cp_usb *
cp_usb_open(void);

void
cp_usb_close(struct cp_usb *cp);

void
cp_usb_write(struct cp_usb *cp, uint8_t mask, uint8_t value);

uint8_t
cp_usb_read(struct cp_usb *cp);


#endif

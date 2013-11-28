/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_SELFLOAD_H_
#define _AO_SELFLOAD_H_

#include <stdbool.h>
#include "ao-hex.h"
#include "cc-usb.h"

struct ao_hex_image *
ao_self_read(struct cc_usb *cc, uint32_t address, uint32_t length);

bool
ao_self_write(struct cc_usb *cc, struct ao_hex_image *image);

uint16_t
ao_self_get_uint16(struct cc_usb *cc, uint32_t addr);

uint32_t
ao_self_get_uint32(struct cc_usb *cc, uint32_t addr);

#endif /* _AO_SELFLOAD_H_ */

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

/*
 * For hardware without eeprom, the config code still
 * wants to call these functions
 */
uint8_t
ao_ee_write_config(uint8_t *buf, uint16_t len) __reentrant
{
	(void) buf;
	(void) len;
	return 1;
}

uint8_t
ao_ee_read_config(uint8_t *buf, uint16_t len) __reentrant
{
	memset(buf, '\0', len);
	return 1;
}

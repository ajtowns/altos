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

#ifndef _AO_BOOT_H_
#define _AO_BOOT_H_

void
ao_boot_chain(uint32_t *base);

void
ao_boot_check_pin(void);

/* Return true to switch to application (if present) */
int
ao_boot_check_chain(void);

void
ao_boot_reboot(uint32_t *base);

#define AO_BOOT_FORCE_LOADER	((uint32_t *) 0)

static inline void
ao_boot_loader(void) {
	ao_boot_reboot(AO_BOOT_FORCE_LOADER);
}

#endif /* _AO_BOOT_H_ */

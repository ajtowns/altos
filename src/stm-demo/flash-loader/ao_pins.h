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

#ifndef _AO_PINS_H_
#define _AO_PINS_H_

/* Bridge SB17 on the board and use the MCO from the other chip */
#define AO_HSE		8000000
#define AO_HSE_BYPASS		1

#include <ao_flash_stm_pins.h>

/* Use the 'user switch' to force boot loader on power on */

#define AO_BOOT_PIN			1
#define AO_BOOT_APPLICATION_GPIO	stm_gpioa
#define AO_BOOT_APPLICATION_PIN		0
#define AO_BOOT_APPLICATION_VALUE	0
#define AO_BOOT_APPLICATION_MODE	0

#endif /* _AO_PINS_H_ */

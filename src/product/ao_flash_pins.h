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

#ifndef _AO_FLASH_PINS_H_
#define _AO_FLASH_PINS_H_

/* Common definitions for the USB flash loader */

#define HAS_TASK_QUEUE		0

#define HAS_USB			1
#define USE_USB_STDIO		0
#define HAS_BEEP		0
#define HAS_TASK		0
#define HAS_ECHO		0
#ifndef HAS_TICK
#define HAS_TICK		0
#endif

#define PACKET_HAS_SLAVE	0

#define HAS_TASK_INFO		0
#define HAS_VERSION		0

#define AO_BOOT_CHAIN		1

#define IS_FLASH_LOADER		1

#endif /* _AO_FLASH_PINS_H_ */

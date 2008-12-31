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

#ifndef _CCDBG_DEBUG_H_
#define _CCDBG_DEBUG_H_
/* Debug levels
 */
#define CC_DEBUG_BITBANG	0x00000001
#define CC_DEBUG_COMMAND	0x00000002
#define CC_DEBUG_INSTRUCTIONS	0x00000004
#define CC_DEBUG_EXECUTE	0x00000008
#define CC_DEBUG_FLASH		0x00000010
#define CC_DEBUG_MEMORY		0x00000020
#define CC_DEBUG_USB_ASYNC	0x00000040

/* ccdbg-debug.c */
void
ccdbg_debug(int level, char *format, ...);

void
ccdbg_add_debug(int level);

void
ccdbg_clear_debug(int level);

void
ccdbg_flush(int level);

#endif /* _CCDBG_DEBUG_H_ */

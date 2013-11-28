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

#ifndef _AO_VERBOSE_H_
#define _AO_VERBOSE_H_

#include <stdint.h>
#include <stdarg.h>

uint32_t	ao_verbose;

#define AO_VERBOSE_EXE	1
#define AO_VERBOSE_SELF	2

void
ao_printf(uint32_t verbose, const char *format, ...);

#endif /* _AO_VERBOSE_H_ */

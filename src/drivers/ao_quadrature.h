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

#ifndef _AO_QUADRATURE_H_
#define _AO_QUADRATURE_H_

extern __xdata int32_t ao_quadrature_count[AO_QUADRATURE_COUNT];

int32_t
ao_quadrature_wait(uint8_t q);

int32_t
ao_quadrature_poll(uint8_t q);

void
ao_quadrature_init(void);

#endif /* _AO_QUADRATURE_H_ */

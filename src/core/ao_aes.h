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

#ifndef _AO_AES_H_
#define _AO_AES_H_

/* ao_aes.c */

extern __xdata uint8_t ao_aes_mutex;

/* AES keys and blocks are 128 bits */

enum ao_aes_mode {
	ao_aes_mode_cbc_mac
};

#if HAS_AES
#ifdef SDCC
void
ao_aes_isr(void) __interrupt 4;
#endif
#endif

void
ao_aes_set_mode(enum ao_aes_mode mode);

void
ao_aes_set_key(__xdata uint8_t *in);

void
ao_aes_zero_iv(void);

void
ao_aes_run(__xdata uint8_t *in,
	   __xdata uint8_t *out);

void
ao_aes_init(void);

#endif /* _AO_AES_H_ */

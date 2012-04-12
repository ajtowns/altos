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

#ifndef _AO_DBG_H_
#define _AO_DBG_H_

/*
 * ao_dbg.c
 *
 * debug another telemetrum board
 */

/* Send a byte to the dbg target */
void
ao_dbg_send_byte(uint8_t byte);

/* Receive a byte from the dbg target */
uint8_t
ao_dbg_recv_byte(void);

/* Start a bulk transfer to/from dbg target memory */
void
ao_dbg_start_transfer(uint16_t addr);

/* End a bulk transfer to/from dbg target memory */
void
ao_dbg_end_transfer(void);

/* Write a byte to dbg target memory */
void
ao_dbg_write_byte(uint8_t byte);

/* Read a byte from dbg target memory */
uint8_t
ao_dbg_read_byte(void);

/* Enable dbg mode, switching use of the pins */
void
ao_dbg_debug_mode(void);

/* Reset the dbg target */
void
ao_dbg_reset(void);

void
ao_dbg_init(void);

#endif /* _AO_DBG_H_ */

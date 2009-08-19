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

#include "ccdbg.h"
#include <time.h>
#include "cc-usb.h"
#include "cc-bitbang.h"

struct ccdbg *
ccdbg_open(char *tty)
{
	struct ccdbg *dbg;

	dbg = calloc(sizeof (struct ccdbg), 1);
	if (!dbg) {
		perror("calloc");
		return NULL;
	}
	if (!tty)
		tty = getenv("ALTOS_TTY");
	if (!tty)
		tty="/dev/ttyACM0";

	if (!strcmp(tty, "BITBANG")) {
		dbg->bb = cc_bitbang_open();
		if (!dbg->bb) {
			free(dbg);
			return NULL;
		}
	} else {
		dbg->usb = cc_usb_open(tty);
		if (!dbg->usb) {
			free(dbg);
			return NULL;
		}
	}
	return dbg;
}

void
ccdbg_close(struct ccdbg *dbg)
{
	if (dbg->usb)
		cc_usb_close(dbg->usb);
	if (dbg->bb)
		cc_bitbang_close(dbg->bb);
	free (dbg);
}

void
ccdbg_debug_mode(struct ccdbg *dbg)
{
	if (dbg->usb)
		cc_usb_debug_mode(dbg->usb);
	else if (dbg->bb)
		cc_bitbang_debug_mode(dbg->bb);
}

void
ccdbg_reset(struct ccdbg *dbg)
{
	if (dbg->usb)
		cc_usb_reset(dbg->usb);
	else if (dbg->bb)
		cc_bitbang_reset(dbg->bb);
}

void
ccdbg_send_bytes(struct ccdbg *dbg, uint8_t *bytes, int nbytes)
{
	if (dbg->usb)
		cc_usb_send_bytes(dbg->usb, bytes, nbytes);
	else if (dbg->bb)
		cc_bitbang_send_bytes(dbg->bb, bytes, nbytes);
}

void
ccdbg_recv_bytes(struct ccdbg *dbg, uint8_t *bytes, int nbytes)
{
	if (dbg->usb)
		cc_usb_recv_bytes(dbg->usb, bytes, nbytes);
	else if (dbg->bb)
		cc_bitbang_recv_bytes(dbg->bb, bytes, nbytes);
}

void
ccdbg_sync(struct ccdbg *dbg)
{
	if (dbg->usb)
		cc_usb_sync(dbg->usb);
	else if (dbg->bb)
		cc_bitbang_sync(dbg->bb);
}

void
ccdbg_cmd_write(struct ccdbg *dbg, uint8_t cmd, uint8_t *data, int len)
{
	ccdbg_send_bytes(dbg, &cmd, 1);
	ccdbg_send_bytes(dbg, data, len);
}

uint8_t
ccdbg_cmd_write_read8(struct ccdbg *dbg, uint8_t cmd, uint8_t *data, int len)
{
	uint8_t	byte[1];
	ccdbg_cmd_write(dbg, cmd, data, len);
	ccdbg_recv_bytes(dbg, byte, 1);
	ccdbg_sync(dbg);
	return byte[0];
}

uint16_t
ccdbg_cmd_write_read16(struct ccdbg *dbg, uint8_t cmd, uint8_t *data, int len)
{
	uint8_t	byte[2];
	ccdbg_cmd_write(dbg, cmd, data, len);
	ccdbg_recv_bytes(dbg, byte, 2);
	ccdbg_sync(dbg);
	return (byte[0] << 8) | byte[1];
}

void
ccdbg_cmd_write_queue8(struct ccdbg *dbg, uint8_t cmd,
		       uint8_t *data, int len,
		       uint8_t *reply)
{
	ccdbg_cmd_write(dbg, cmd, data, len);
	ccdbg_recv_bytes(dbg, reply, 1);
}

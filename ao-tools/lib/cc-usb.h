/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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

#ifndef _CC_USB_H_
#define _CC_USB_H_

#include <stdint.h>

struct cc_usb;

struct cc_usb *
cc_usb_open(char *tty);

void
cc_usb_close(struct cc_usb *cc);

int
cc_usb_send_bytes(struct cc_usb *cc, uint8_t *bytes, int len);

int
cc_usb_recv_bytes(struct cc_usb *cc, uint8_t *bytes, int len);

int
cc_usb_write_memory(struct cc_usb *cc, uint16_t addr, uint8_t *bytes, int len);

int
cc_usb_read_memory(struct cc_usb *cc, uint16_t addr, uint8_t *bytes, int len);

int
cc_usb_debug_mode(struct cc_usb *cc);

int
cc_usb_reset(struct cc_usb *cc);

void
cc_usb_sync(struct cc_usb *cc);

void
cc_queue_read(struct cc_usb *cc, uint8_t *buf, int len);

int
cc_usb_getchar_timeout(struct cc_usb *cc, int timeout);

int
cc_usb_getchar(struct cc_usb *cc);

void
cc_usb_getline(struct cc_usb *cc, char *line, int max);

void
cc_usb_printf(struct cc_usb *cc, char *format, ...);

int
cc_usb_write(struct cc_usb *cc, void *buf, int c);

void
cc_usb_open_remote(struct cc_usb *cc, int freq, char *call);

void
cc_usb_close_remote(struct cc_usb *cc);

#endif /* _CC_USB_H_ */

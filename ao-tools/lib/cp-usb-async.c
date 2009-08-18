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

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include "cp-usb-async.h"
#include "ccdbg-debug.h"

#define MAX_OUTSTANDING		256
#define CP_TIMEOUT		1000	/* ms */

struct cp_usb_packet {
	struct libusb_transfer	*transfer;
	enum { packet_read, packet_write } direction;
	unsigned char		data[9];
	uint8_t			*valuep;
};

struct cp_usb_async {
	libusb_context		*ctx;
	libusb_device_handle	*handle;
	struct cp_usb_packet	packet[MAX_OUTSTANDING];
	int			p, ack;
	uint8_t			value;
	uint8_t			set;
};

struct cp_usb_async *
cp_usb_async_open(void)
{
	struct cp_usb_async *cp;
	int ret;

	cp = calloc(sizeof (struct cp_usb_async), 1);
	if (!cp)
		return NULL;
	ret = libusb_init(&cp->ctx);
	if (ret) {
		free(cp);
		return NULL;
	}
	cp->handle = libusb_open_device_with_vid_pid(cp->ctx,
						     0x10c4, 0xea60);
	cp->ack = -1;
	if (!cp->handle) {
		fprintf(stderr, "Cannot find USB device 10c4:ea60\n");
		libusb_exit(cp->ctx);
		free(cp);
		return NULL;
	}
	cp->value = 0;
	cp->set = 0;
	return cp;
}

void
cp_usb_async_close(struct cp_usb_async *cp)
{
	libusb_close(cp->handle);
	libusb_exit(cp->ctx);
	free(cp);
}

static void
cp_usb_async_transfer_callback(struct libusb_transfer *transfer)
{
	struct cp_usb_async *cp = transfer->user_data;
	int p;

	for (p = 0; p < cp->p; p++)
		if (cp->packet[p].transfer == transfer)
			break;
	if (p == cp->p) {
		fprintf(stderr, "unknown transfer\n");
		return;
	}
	switch (cp->packet[p].direction) {
	case packet_read:
		ccdbg_debug(CC_DEBUG_USB_ASYNC, "ack read %d 0x%02x\n",
			    p, cp->packet[p].data[8]);
		*cp->packet[p].valuep = cp->packet[p].data[8];
		break;
	case packet_write:
		ccdbg_debug(CC_DEBUG_USB_ASYNC, "ack write %d\n", p);
		break;
	}
	if (p > cp->ack)
		cp->ack = p;
}

void
cp_usb_async_write(struct cp_usb_async *cp, uint8_t mask, uint8_t value)
{
	int	p;
	uint16_t gpio_set;
	int	ret;

	if (cp->set) {
		value = (cp->value & ~mask) | (value & mask);
		mask = value ^ cp->value;
	}
	cp->set = 1;
	cp->value = value;
	gpio_set = ((uint16_t) value << 8) | mask;
	if (cp->p == MAX_OUTSTANDING)
		cp_usb_async_sync(cp);
	p = cp->p;
	if (!cp->packet[p].transfer)
		cp->packet[p].transfer = libusb_alloc_transfer(0);
	cp->packet[p].direction = packet_write;
	libusb_fill_control_setup(cp->packet[p].data,
				  0x40,			/* request */
				  0xff,			/* request type */
				  0x37e1,		/* value */
				  gpio_set,		/* index */
				  0);			/* length */

	libusb_fill_control_transfer(cp->packet[p].transfer,
				     cp->handle,
				     cp->packet[p].data,
				     cp_usb_async_transfer_callback,
				     cp,
				     CP_TIMEOUT);
	ccdbg_debug(CC_DEBUG_USB_ASYNC, "Write packet %d 0x%x 0x%x\n", p, mask, value);
	ret = libusb_submit_transfer(cp->packet[p].transfer);
	if (ret)
		fprintf(stderr, "libusb_submit_transfer failed %d\n", ret);
	cp->p++;
}

void
cp_usb_async_read(struct cp_usb_async *cp, uint8_t *valuep)
{
	int	p;
	int	ret;

	if (cp->p == MAX_OUTSTANDING)
		cp_usb_async_sync(cp);
	p = cp->p;
	if (!cp->packet[p].transfer)
		cp->packet[p].transfer = libusb_alloc_transfer(0);
	cp->packet[p].valuep = valuep;
	cp->packet[p].direction = packet_read;
	libusb_fill_control_setup(cp->packet[p].data,
				  0xc0,			/* request */
				  0xff,			/* request type */
				  0x00c2,		/* value */
				  0,			/* index */
				  1);			/* length */

	libusb_fill_control_transfer(cp->packet[p].transfer,
				     cp->handle,
				     cp->packet[p].data,
				     cp_usb_async_transfer_callback,
				     cp,
				     CP_TIMEOUT);
	ccdbg_debug(CC_DEBUG_USB_ASYNC, "Read packet %d\n", p);
	ret = libusb_submit_transfer(cp->packet[p].transfer);
	if (ret)
		fprintf(stderr, "libusb_submit_transfer failed %d\n", ret);
	cp->p++;
}

void
cp_usb_async_sync(struct cp_usb_async *cp)
{
	while (cp->ack < cp->p - 1) {
		libusb_handle_events(cp->ctx);
	}
	cp->p = 0;
	cp->ack = -1;
}

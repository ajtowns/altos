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
#include <usb.h>

#define CP2101_UART	0x00
#define UART_ENABLE	0x0001
#define UART_DISABLE	0x0000
#define REQTYPE_HOST_TO_DEVICE  0x41
#define REQTYPE_DEVICE_TO_HOST  0xc1

static int
cp_usb_gpio_get(struct ccdbg *dbg, uint8_t *gpio_get)
{
	return usb_control_msg(dbg->usb_dev,		/* dev */
			       0xc0,			/* request */
			       0xff,			/* requesttype */
			       0x00c2,			/* value */
			       0,			/* index */
			       (char *) gpio_get,	/* bytes */
			       1,			/* size */
			       300);			/* timeout */
}

static int
cp_usb_gpio_set(struct ccdbg *dbg, uint8_t mask, uint8_t value)
{
	uint16_t gpio_set = ((uint16_t) value << 8) | mask;

	return usb_control_msg(dbg->usb_dev,		/* dev */
			       0x40,			/* request */
			       0xff,			/* requesttype */
			       0x37e1,			/* value */
			       gpio_set,		/* index */
			       NULL,			/* bytes */
			       0,			/* size */
			       300);			/* timeout */
}

static int
cp_usb_uart_enable_disable(struct ccdbg *dbg, uint16_t enable)
{
	return usb_control_msg(dbg->usb_dev,
			       CP2101_UART,
			       REQTYPE_HOST_TO_DEVICE,
			       enable,
			       0,
			       NULL,
			       0,
			       300);
}

void
cp_usb_init(struct ccdbg *dbg)
{
	usb_dev_handle *dev_handle;
	struct usb_device *dev = NULL;
	struct usb_bus *bus, *busses;
	int interface;
	int ret;
	uint8_t gpio;
	
	usb_init();
	usb_find_busses();
	usb_find_devices();
	
	busses = usb_get_busses();
	for (bus = busses; bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			if (dev->descriptor.idVendor == 0x10c4 &&
			    dev->descriptor.idProduct == 0xea60)
				break;
		}
		if (dev)
			break;
	}
	if (!dev){
		perror("No CP2103 found");
		exit(1);
	}
	interface = 0;
	dev_handle = usb_open(dev);
	usb_detach_kernel_driver_np(dev_handle, interface);
	usb_claim_interface(dev_handle, interface);
	dbg->usb_dev = dev_handle;
	ret = cp_usb_uart_enable_disable(dbg, UART_DISABLE);
	dbg->gpio = 0xf;
	ret = cp_usb_gpio_set(dbg, 0xf, dbg->gpio);
	ret = cp_usb_gpio_get(dbg, &gpio);
}

void
cp_usb_fini(struct ccdbg *dbg)
{
	cp_usb_uart_enable_disable(dbg, UART_DISABLE);
	usb_close(dbg->usb_dev);
}

void
cp_usb_write(struct ccdbg *dbg, uint8_t mask, uint8_t value)
{
	uint8_t	new_gpio;
	int ret;

	new_gpio = (dbg->gpio & ~mask) | (value & mask);
	if (new_gpio != dbg->gpio) {
		ret = cp_usb_gpio_set(dbg, new_gpio ^ dbg->gpio, new_gpio);
		if (ret < 0)
			perror("gpio_set");
		dbg->gpio = new_gpio;
	}
}

uint8_t
cp_usb_read(struct ccdbg *dbg)
{
	int ret;
	uint8_t gpio;

	ret = cp_usb_gpio_get(dbg, &gpio);
	if (ret < 0)
		perror("gpio_set");
	return gpio;
}

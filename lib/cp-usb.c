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

/*
 * libusb interface to the GPIO pins on a CP2103.
 *
 * Various magic constants came from the cp210x driver published by silabs.
 */

#include "cp-usb.h"
#include <stdio.h>
#include <errno.h>
#include <libusb.h>

struct cp_usb {
	usb_dev_handle *usb_dev;
	uint8_t	gpio;
};

#define CP2101_UART	0x00
#define UART_ENABLE	0x0001
#define UART_DISABLE	0x0000
#define REQTYPE_HOST_TO_DEVICE  0x41
#define REQTYPE_DEVICE_TO_HOST  0xc1

static int
cp_usb_gpio_get(struct cp_usb *cp, uint8_t *gpio_get)
{
	return usb_control_msg(cp->usb_dev,		/* dev */
			       0xc0,			/* request */
			       0xff,			/* requesttype */
			       0x00c2,			/* value */
			       0,			/* index */
			       (char *) gpio_get,	/* bytes */
			       1,			/* size */
			       300);			/* timeout */
}

static int
cp_usb_gpio_set(struct cp_usb *cp, uint8_t mask, uint8_t value)
{
	uint16_t gpio_set = ((uint16_t) value << 8) | mask;

	return usb_control_msg(cp->usb_dev,		/* dev */
			       0x40,			/* request */
			       0xff,			/* requesttype */
			       0x37e1,			/* value */
			       gpio_set,		/* index */
			       NULL,			/* bytes */
			       0,			/* size */
			       300);			/* timeout */
}

static int
cp_usb_uart_enable_disable(struct cp_usb *cp, uint16_t enable)
{
	return usb_control_msg(cp->usb_dev,
			       CP2101_UART,
			       REQTYPE_HOST_TO_DEVICE,
			       enable,
			       0,
			       NULL,
			       0,
			       300);
}

struct cp_usb *
cp_usb_open(void)
{
	struct cp_usb *cp;
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
		return NULL;
	}
	cp = calloc(sizeof(struct cp_usb), 1);
	interface = 0;
	dev_handle = usb_open(dev);
	usb_detach_kernel_driver_np(dev_handle, interface);
	usb_claim_interface(dev_handle, interface);
	cp->usb_dev = dev_handle;
	ret = cp_usb_uart_enable_disable(cp, UART_DISABLE);
	cp->gpio = 0xf;
	ret = cp_usb_gpio_set(cp, 0xf, cp->gpio);
	ret = cp_usb_gpio_get(cp, &gpio);
	return cp;
}

void
cp_usb_close(struct cp_usb *cp)
{
	cp_usb_uart_enable_disable(cp, UART_DISABLE);
	usb_close(cp->usb_dev);
	free(cp);
}

void
cp_usb_write(struct cp_usb *cp, uint8_t mask, uint8_t value)
{
	uint8_t	new_gpio;
	int ret;

	new_gpio = (cp->gpio & ~mask) | (value & mask);
	if (new_gpio != cp->gpio) {
		ret = cp_usb_gpio_set(cp, new_gpio ^ cp->gpio, new_gpio);
		if (ret < 0)
			perror("gpio_set");
		cp->gpio = new_gpio;
	}
}

uint8_t
cp_usb_read(struct cp_usb *cp)
{
	int ret;
	uint8_t gpio;

	ret = cp_usb_gpio_get(cp, &gpio);
	if (ret < 0)
		perror("gpio_get");
	return gpio;
}

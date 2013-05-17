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

#ifndef _AO_USB_H_
#define _AO_USB_H_

/*
 * ao_usb.c
 */

/* Put one character to the USB output queue */
void
ao_usb_putchar(char c);

/* Get one character from the USB input queue */
char
ao_usb_getchar(void);

/* Poll for a charcter on the USB input queue.
 * returns AO_READ_AGAIN if none are available
 */
int
ao_usb_pollchar(void);

/* Flush the USB output queue */
void
ao_usb_flush(void);

/* Enable the USB controller */
void
ao_usb_enable(void);

/* Disable the USB controller */
void
ao_usb_disable(void);

/* Initialize the USB system */
void
ao_usb_init(void);

extern __code __at (0x00aa) uint8_t ao_usb_descriptors [];

#define AO_USB_SETUP_DIR_MASK	(0x01 << 7)
#define AO_USB_SETUP_TYPE_MASK	(0x03 << 5)
#define AO_USB_SETUP_RECIP_MASK	(0x1f)

#define AO_USB_DIR_OUT			0
#define AO_USB_DIR_IN			(1 << 7)

#define AO_USB_TYPE_STANDARD		0
#define AO_USB_TYPE_CLASS		(1 << 5)
#define AO_USB_TYPE_VENDOR		(2 << 5)
#define AO_USB_TYPE_RESERVED		(3 << 5)

#define AO_USB_RECIP_DEVICE		0
#define AO_USB_RECIP_INTERFACE		1
#define AO_USB_RECIP_ENDPOINT		2
#define AO_USB_RECIP_OTHER		3

/* standard requests */
#define	AO_USB_REQ_GET_STATUS		0x00
#define AO_USB_REQ_CLEAR_FEATURE	0x01
#define AO_USB_REQ_SET_FEATURE		0x03
#define AO_USB_REQ_SET_ADDRESS		0x05
#define AO_USB_REQ_GET_DESCRIPTOR	0x06
#define AO_USB_REQ_SET_DESCRIPTOR	0x07
#define AO_USB_REQ_GET_CONFIGURATION	0x08
#define AO_USB_REQ_SET_CONFIGURATION	0x09
#define AO_USB_REQ_GET_INTERFACE	0x0A
#define AO_USB_REQ_SET_INTERFACE	0x0B
#define AO_USB_REQ_SYNCH_FRAME		0x0C

#define AO_USB_DESC_DEVICE		1
#define AO_USB_DESC_CONFIGURATION	2
#define AO_USB_DESC_STRING		3
#define AO_USB_DESC_INTERFACE		4
#define AO_USB_DESC_ENDPOINT		5
#define AO_USB_DESC_DEVICE_QUALIFIER	6
#define AO_USB_DESC_OTHER_SPEED		7
#define AO_USB_DESC_INTERFACE_POWER	8

#define AO_USB_GET_DESC_TYPE(x)		(((x)>>8)&0xFF)
#define AO_USB_GET_DESC_INDEX(x)	((x)&0xFF)

#define AO_USB_CONTROL_EP	0
#define AO_USB_CONTROL_SIZE	32

#define AO_USB_INT_EP		1
#define AO_USB_INT_SIZE		8

#define AO_USB_OUT_EP		4
#define AO_USB_IN_EP		5
/*
 * USB bulk packets can only come in 8, 16, 32 and 64
 * byte sizes, so we'll use 64 for everything
 */
#define AO_USB_IN_SIZE		64
#define AO_USB_OUT_SIZE		64

#define AO_USB_EP0_IDLE		0
#define AO_USB_EP0_DATA_IN	1
#define AO_USB_EP0_DATA_OUT	2
#define AO_USB_EP0_STALL	3

#define LE_WORD(x)    ((x)&0xFF),((uint8_t) (((uint16_t) (x))>>8))

/* CDC definitions */
#define AO_USB_CS_INTERFACE      	0x24
#define AO_USB_CS_ENDPOINT       	0x25

#define AO_USB_SET_LINE_CODING		0x20
#define AO_USB_GET_LINE_CODING		0x21
#define AO_USB_SET_CONTROL_LINE_STATE	0x22

/* Data structure for GET_LINE_CODING / SET_LINE_CODING class requests */
struct ao_usb_line_coding {
	uint32_t	rate;
	uint8_t		char_format;
	uint8_t		parity;
	uint8_t		data_bits;
} ;

#endif /* _AO_USB_H_ */

/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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

#define AO_USB_DIR_OUT			0
#define AO_USB_DIR_IN			1

#define AO_USB_TYPE_STANDARD		0
#define AO_USB_TYPE_CLASS		1
#define AO_USB_TYPE_VENDOR		2
#define AO_USB_TYPE_RESERVED		3

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

#endif /* _AO_USB_H_ */

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

#include "ao.h"
#include "ao_usb.h"

struct ao_task __xdata ao_usb_task;

#define AO_USB_CONTROL_EP	0
#define AO_USB_INT_EP		1
#define AO_USB_OUT_EP		4
#define AO_USB_IN_EP		5
#define AO_USB_CONTROL_SIZE	32
/*
 * Double buffer IN and OUT EPs, so each
 * gets half of the available space
 */
#define AO_USB_IN_SIZE		256
#define AO_USB_OUT_SIZE		128

static __xdata uint16_t	ao_usb_in_bytes;
static __xdata uint16_t	ao_usb_out_bytes;
static __xdata uint8_t	ao_usb_iif;
static __xdata uint8_t	ao_usb_oif;

/* This interrupt is shared with port 2, 
 * so when we hook that up, fix this
 */
void
ao_usb_isr(void) interrupt 6
{
	USBIF = 0;
	ao_usb_iif |= USBIIF;
	if (ao_usb_iif & 1)
		ao_wakeup(&ao_usb_task);
	if (ao_usb_iif & (1 << AO_USB_IN_EP))
		ao_wakeup(&ao_usb_in_bytes);

	ao_usb_oif |= USBOIF;
	if (ao_usb_oif & (1 << AO_USB_OUT_EP))
		ao_wakeup(&ao_usb_out_bytes);
}

#define AO_USB_EP0_IDLE		0
#define AO_USB_EP0_DATA_IN	1
#define AO_USB_EP0_DATA_OUT	2

struct ao_usb_setup {
	uint8_t		dir_type_recip;
	uint8_t		request;
	uint16_t	value;
	uint16_t	index;
	uint16_t	length;
} __xdata ao_usb_setup;

__xdata uint8_t ao_usb_ep0_state;
uint8_t * __xdata ao_usb_ep0_in_data;
__xdata uint8_t ao_usb_ep0_in_len;
__xdata uint8_t	ao_usb_ep0_in_buf[2];
__xdata uint8_t ao_usb_ep0_out_len;
__xdata uint8_t *__data ao_usb_ep0_out_data;
__xdata uint8_t ao_usb_configuration;

/* Send an IN data packet */
static void
ao_usb_ep0_flush(void)
{
	__xdata uint8_t this_len;
	__xdata uint8_t	cs0;
	
	USBINDEX = 0;
	cs0 = USBCS0;
	if (cs0 & USBCS0_INPKT_RDY)
		ao_panic(0);

	this_len = ao_usb_ep0_in_len;
	if (this_len > AO_USB_CONTROL_SIZE)
		this_len = AO_USB_CONTROL_SIZE;
	cs0 = USBCS0_INPKT_RDY;
	if (this_len != AO_USB_CONTROL_SIZE) {
		cs0 = USBCS0_INPKT_RDY | USBCS0_DATA_END;
		ao_usb_ep0_state = AO_USB_EP0_IDLE;
	}
	ao_usb_ep0_in_len -= this_len;
	while (this_len--)
		USBFIFO[0] = *ao_usb_ep0_in_data++;
	USBINDEX = 0;
	USBCS0 = cs0;
}

#define LE_WORD(x)    ((x)&0xFF),((uint8_t) (((uint16_t) (x))>>8))

/* CDC definitions */
#define CS_INTERFACE      0x24
#define CS_ENDPOINT       0x25

#define SET_LINE_CODING         0x20
#define GET_LINE_CODING         0x21
#define SET_CONTROL_LINE_STATE  0x22

/* Data structure for GET_LINE_CODING / SET_LINE_CODING class requests */
struct ao_usb_line_coding {
	uint32_t	rate;
	uint8_t		char_format;
	uint8_t		parity;
	uint8_t		data_bits;
} ;

__xdata static struct ao_usb_line_coding ao_usb_line_coding = {115200, 0, 0, 8};

/* USB descriptors in one giant block of bytes */
static const uint8_t ao_usb_descriptors [] = 
{
	/* Device descriptor */
	0x12,
	AO_USB_DESC_DEVICE,
	LE_WORD(0x0110),	/*  bcdUSB */
	0x02,			/*  bDeviceClass */
	0x00,			/*  bDeviceSubClass */
	0x00,			/*  bDeviceProtocol */
	AO_USB_CONTROL_SIZE,	/*  bMaxPacketSize */
	LE_WORD(0xFFFE),	/*  idVendor */
	LE_WORD(0x000A),	/*  idProduct */
	LE_WORD(0x0100),	/*  bcdDevice */
	0x01,			/*  iManufacturer */
	0x02,			/*  iProduct */
	0x03,			/*  iSerialNumber */
	0x01,			/*  bNumConfigurations */

	/* Configuration descriptor */
	0x09,
	AO_USB_DESC_CONFIGURATION,
	LE_WORD(67),		/*  wTotalLength */
	0x02,			/*  bNumInterfaces */
	0x01,			/*  bConfigurationValue */
	0x00,			/*  iConfiguration */
	0xC0,			/*  bmAttributes */
	0x32,			/*  bMaxPower */

	/* Control class interface */
	0x09,
	AO_USB_DESC_INTERFACE,
	0x00,			/*  bInterfaceNumber */
	0x00,			/*  bAlternateSetting */
	0x01,			/*  bNumEndPoints */
	0x02,			/*  bInterfaceClass */
	0x02,			/*  bInterfaceSubClass */
	0x01,			/*  bInterfaceProtocol, linux requires value of 1 for the cdc_acm module */
	0x00,			/*  iInterface */

	/* Header functional descriptor */
	0x05,
	CS_INTERFACE,
	0x00,			/*  bDescriptor SubType Header */
	LE_WORD(0x0110),	/*  CDC version 1.1 */

	/* Call management functional descriptor */
	0x05,
	CS_INTERFACE,
	0x01,			/* bDescriptor SubType Call Management */
	0x01,			/* bmCapabilities = device handles call management */
	0x01,			/* bDataInterface call management interface number */

	/* ACM functional descriptor */
	0x04,
	CS_INTERFACE,
	0x02,			/* bDescriptor SubType Abstract Control Management */
	0x02,			/* bmCapabilities = D1 (Set_line_Coding, Set_Control_Line_State, Get_Line_Coding and Serial_State) */

	/* Union functional descriptor */
	0x05,
	CS_INTERFACE,
	0x06,			/* bDescriptor SubType Union Functional descriptor */
	0x00,			/* bMasterInterface */
	0x01,			/* bSlaveInterface0 */

	/* Notification EP */
	0x07,
	AO_USB_DESC_ENDPOINT,
	AO_USB_INT_EP|0x80,	/* bEndpointAddress */
	0x03,			/* bmAttributes = intr */
	LE_WORD(8),		/* wMaxPacketSize */
	0x0A,			/* bInterval */

	/* Data class interface descriptor */
	0x09,
	AO_USB_DESC_INTERFACE,
	0x01,			/* bInterfaceNumber */
	0x00,			/* bAlternateSetting */
	0x02,			/* bNumEndPoints */
	0x0A,			/* bInterfaceClass = data */
	0x00,			/* bInterfaceSubClass */
	0x00,			/* bInterfaceProtocol */
	0x00,			/* iInterface */

	/* Data EP OUT */
	0x07,
	AO_USB_DESC_ENDPOINT,
	AO_USB_OUT_EP,		/* bEndpointAddress */
	0x02,			/* bmAttributes = bulk */
	LE_WORD(AO_USB_OUT_SIZE),/* wMaxPacketSize */
	0x00,			/* bInterval */

	/* Data EP in */
	0x07,
	AO_USB_DESC_ENDPOINT,
	AO_USB_IN_EP|0x80,	/* bEndpointAddress */
	0x02,			/* bmAttributes = bulk */
	LE_WORD(AO_USB_IN_SIZE),/* wMaxPacketSize */
	0x00,			/* bInterval */

	/* String descriptors */
	0x04,
	AO_USB_DESC_STRING,
	LE_WORD(0x0409),

	/* iManufacturer */
	0x20,
	AO_USB_DESC_STRING,
	'a', 0, 'l', 0, 't', 0, 'u', 0, 's', 0, 'm', 0, 'e', 0, 't', 0, 'r', 0, 'u', 0, 'm', 0, '.', 0, 'o', 0, 'r', 0, 'g', 0, 

	/* iProduct */
	0x16,
	AO_USB_DESC_STRING,
	'T', 0, 'e', 0, 'l', 0, 'e', 0, 'M', 0, 'e', 0, 't', 0, 'r', 0, 'u', 0, 'm', 0, 

	/* iSerial */
	0x0e,
	AO_USB_DESC_STRING,
	'0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, 

	/* Terminating zero */
	0
};

/* Walk through the list of descriptors and find a match
 */
static void
ao_usb_get_descriptor(uint16_t value)
{
	const uint8_t		*__xdata descriptor;
	__xdata uint8_t		type = value >> 8;
	__xdata uint8_t		index = value;

	descriptor = ao_usb_descriptors;
	while (descriptor[0] != 0) {
		if (descriptor[1] == type && index-- == 0) {
			if (type == AO_USB_DESC_CONFIGURATION)
				ao_usb_ep0_in_len = descriptor[2];
			else
				ao_usb_ep0_in_len = descriptor[0];
			ao_usb_ep0_in_data = descriptor;
			break;
		}
		descriptor += descriptor[0];
	}
}

/* Read data from the ep0 OUT fifo
 */
static void
ao_usb_ep0_fill(void)
{
	__xdata uint8_t	len;
	
	USBINDEX = 0;
	len = USBCNT0;
	if (len > ao_usb_ep0_out_len)
		len = ao_usb_ep0_out_len;
	ao_usb_ep0_out_len -= len;
	while (len--)
		*ao_usb_ep0_out_data++ = USBFIFO[0];
}

void
ao_usb_ep0_queue_byte(uint8_t a)
{
	ao_usb_ep0_in_buf[ao_usb_ep0_in_len++] = a;
}

void
ao_usb_set_address(uint8_t address)
{
	USBADDR = address | 0x80;
	while (USBADDR & 0x80)
		;
}

static void
ao_usb_set_configuration(void)
{
	/* Set the IN max packet size, double buffered */
	USBINDEX = AO_USB_IN_EP;
	USBMAXI = AO_USB_IN_SIZE >> 3;
	USBCSIH |= USBCSIH_IN_DBL_BUF;

	/* Set the OUT max packet size, double buffered */
	USBINDEX = AO_USB_OUT_EP;
	USBMAXO = AO_USB_OUT_SIZE >> 3;
	USBCSOH = USBCSOH_OUT_DBL_BUF;
}

static void
ao_usb_ep0_setup(void)
{
	/* Pull the setup packet out of the fifo */
	ao_usb_ep0_out_data = (__xdata uint8_t *) &ao_usb_setup;
	ao_usb_ep0_out_len = 8;
	ao_usb_ep0_fill();
	if (ao_usb_ep0_out_len != 0)
		return;

	/* Figure out how to ACK the setup packet */
	if (ao_usb_setup.dir_type_recip & AO_USB_DIR_IN) {
		if (ao_usb_setup.length)
			ao_usb_ep0_state = AO_USB_EP0_DATA_IN;
		else
			ao_usb_ep0_state = AO_USB_EP0_IDLE;
	} else {
		if (ao_usb_setup.length)
			ao_usb_ep0_state = AO_USB_EP0_DATA_OUT;
		else
			ao_usb_ep0_state = AO_USB_EP0_IDLE;
	}
	USBINDEX = 0;
	if (ao_usb_ep0_state == AO_USB_EP0_IDLE)
		USBCS0 = USBCS0_CLR_OUTPKT_RDY | USBCS0_DATA_END;
	else
		USBCS0 = USBCS0_CLR_OUTPKT_RDY;
	
	ao_usb_ep0_in_data = ao_usb_ep0_in_buf;
	ao_usb_ep0_in_len = 0;
	switch(ao_usb_setup.dir_type_recip & AO_USB_SETUP_TYPE_MASK) {
	case AO_USB_TYPE_STANDARD:
		switch(ao_usb_setup.dir_type_recip & AO_USB_SETUP_RECIP_MASK) {
		case AO_USB_RECIP_DEVICE:
			switch(ao_usb_setup.request) {
			case AO_USB_REQ_GET_STATUS:
				ao_usb_ep0_queue_byte(0);
				ao_usb_ep0_queue_byte(0);
				break;
			case AO_USB_REQ_SET_ADDRESS:
				ao_usb_set_address(ao_usb_setup.value);
				break;
			case AO_USB_REQ_GET_DESCRIPTOR:
				ao_usb_get_descriptor(ao_usb_setup.value);
				break;
			case AO_USB_REQ_GET_CONFIGURATION:
				ao_usb_ep0_queue_byte(ao_usb_configuration);
				break;
			case AO_USB_REQ_SET_CONFIGURATION:
				ao_usb_configuration = ao_usb_setup.value;
				ao_usb_set_configuration();
				break;
			}
			break;
		case AO_USB_RECIP_INTERFACE:
			#pragma disable_warning 110
			switch(ao_usb_setup.request) {
			case AO_USB_REQ_GET_STATUS:
				ao_usb_ep0_queue_byte(0);
				ao_usb_ep0_queue_byte(0);
				break;
			case AO_USB_REQ_GET_INTERFACE:
				ao_usb_ep0_queue_byte(0);
				break;
			case AO_USB_REQ_SET_INTERFACE:
				break;
			}
			break;
		case AO_USB_RECIP_ENDPOINT:
			switch(ao_usb_setup.request) {
			case AO_USB_REQ_GET_STATUS:
				ao_usb_ep0_queue_byte(0);
				ao_usb_ep0_queue_byte(0);
				break;
			}
			break;
		}
		break;
	case AO_USB_TYPE_CLASS:
		switch (ao_usb_setup.request) {
		case SET_LINE_CODING:
			ao_usb_ep0_out_len = 7;
			ao_usb_ep0_out_data = (__xdata uint8_t *) &ao_usb_line_coding;
			break;
		case GET_LINE_CODING:
			ao_usb_ep0_in_len = 7;
			ao_usb_ep0_in_data = (uint8_t *) &ao_usb_line_coding;
			break;
		case SET_CONTROL_LINE_STATE:
			break;
		}
		break;
	}
	if (ao_usb_ep0_state != AO_USB_EP0_DATA_OUT) {
		if (ao_usb_setup.length < ao_usb_ep0_in_len)
			ao_usb_ep0_in_len = ao_usb_setup.length;
		ao_usb_ep0_flush();
	}
}

/* End point 0 receives all of the control messages. */
static void
ao_usb_ep0(void)
{
	__xdata uint8_t	cs0;

	ao_usb_ep0_state = AO_USB_EP0_IDLE;
	for (;;) {
		__critical for (;;) {
			if (ao_usb_iif & 1) {
				ao_usb_iif &= ~1;
				break;
			}
			ao_sleep(&ao_usb_task);
		}
		USBINDEX = 0;
		cs0 = USBCS0;
		if (cs0 & USBCS0_SETUP_END) {
			ao_usb_ep0_state = AO_USB_EP0_IDLE;
			USBCS0 = USBCS0_CLR_SETUP_END;
		}
		if (cs0 & USBCS0_SENT_STALL) {
			ao_usb_ep0_state = AO_USB_EP0_IDLE;
			USBCS0 &= ~USBCS0_SENT_STALL;
		}
		if (ao_usb_ep0_state == AO_USB_EP0_DATA_IN &&
		    (cs0 & USBCS0_INPKT_RDY) == 0)
		{
			ao_usb_ep0_flush();
		}
		if (cs0 & USBCS0_OUTPKT_RDY) {
			switch (ao_usb_ep0_state) {
			case AO_USB_EP0_IDLE:
				ao_usb_ep0_setup();
				break;
			case AO_USB_EP0_DATA_OUT:
				ao_usb_ep0_fill();
				if (ao_usb_ep0_out_len == 0)
					ao_usb_ep0_state = AO_USB_EP0_IDLE;
				USBINDEX = 0;
				if (ao_usb_ep0_state == AO_USB_EP0_IDLE)
					USBCS0 = USBCS0_CLR_OUTPKT_RDY | USBCS0_DATA_END;
				else
					USBCS0 = USBCS0_CLR_OUTPKT_RDY;
				break;
			}
		}
	}
}

void
ao_usb_flush(void) __critical
{
	if (ao_usb_in_bytes) {
		USBINDEX = AO_USB_IN_EP;
		USBCSIL |= USBCSIL_INPKT_RDY;
		ao_usb_in_bytes = 0;
	}
}

void
ao_usb_putchar(uint8_t c) __critical
{
	for (;;) {
		USBINDEX = AO_USB_IN_EP;
		if ((USBCSIL & USBCSIL_INPKT_RDY) == 0)
			break;
		ao_sleep(&ao_usb_in_bytes);
	}
	USBFIFO[AO_USB_IN_EP << 1] = c;
	if (++ao_usb_in_bytes == AO_USB_IN_SIZE) {
		USBINDEX = AO_USB_IN_EP;
		USBCSIL |= USBCSIL_INPKT_RDY;
		ao_usb_in_bytes = 0;
	}
}

uint8_t
ao_usb_getchar(void) __critical
{
	__xdata uint8_t	c;
	while (ao_usb_out_bytes == 0) {
		for (;;) {
			USBINDEX = AO_USB_OUT_EP;
			if ((USBCSOL & USBCSOL_OUTPKT_RDY) != 0)
				break;
			ao_sleep(&ao_usb_out_bytes);
		}
		ao_usb_out_bytes = (USBCNTH << 8) | USBCNTL;
	}
	--ao_usb_out_bytes;
	c = USBFIFO[AO_USB_OUT_EP << 1];
	if (ao_usb_out_bytes == 0) {
		USBINDEX = AO_USB_OUT_EP;
		USBCSOL &= ~USBCSOL_OUTPKT_RDY;
	}
	return c;
}

void
ao_usb_init(void)
{
	/* Turn on the USB controller */
	SLEEP |= SLEEP_USB_EN;

	ao_usb_set_configuration();
	
	/* IN interrupts on the control an IN endpoints */
	USBIIE = (1 << AO_USB_CONTROL_EP) | (1 << AO_USB_IN_EP);

	/* OUT interrupts on the OUT endpoint */
	USBOIE = (1 << AO_USB_OUT_EP);

	/* Ignore control interrupts */
	USBCIE = 0;
	
	/* enable USB interrupts */
	IEN2 |= IEN2_USBIE;

	/* Clear any pending interrupts */
	USBCIF = 0;
	USBOIF = 0;
	USBIIF = 0;
	
	ao_add_task(&ao_usb_task, ao_usb_ep0, "usb");
}

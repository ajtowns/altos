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

static __xdata uint16_t	ao_usb_in_bytes;
static __pdata uint16_t ao_usb_in_bytes_last;
static __xdata uint16_t	ao_usb_out_bytes;
static __pdata uint8_t	ao_usb_iif;
static __pdata uint8_t	ao_usb_running;

static void
ao_usb_set_interrupts(void)
{
	/* IN interrupts on the control an IN endpoints */
	USBIIE = (1 << AO_USB_CONTROL_EP) | (1 << AO_USB_IN_EP);

	/* OUT interrupts on the OUT endpoint */
	USBOIE = (1 << AO_USB_OUT_EP);

	/* Only care about reset */
	USBCIE = USBCIE_RSTIE;
}

/* This interrupt is shared with port 2,
 * so when we hook that up, fix this
 */
void
ao_usb_isr(void) __interrupt 6
{
	USBIF = 0;
	ao_usb_iif |= USBIIF;
	if (ao_usb_iif & 1)
		ao_wakeup(&ao_usb_task);
	if (ao_usb_iif & (1 << AO_USB_IN_EP))
		ao_wakeup(&ao_usb_in_bytes);

	if (USBOIF & (1 << AO_USB_OUT_EP))
		ao_wakeup(&ao_stdin_ready);

	if (USBCIF & USBCIF_RSTIF)
		ao_usb_set_interrupts();
#if HAS_BTM
#if BT_LINK_ON_P2
	ao_btm_isr();
#endif
#endif
}

struct ao_usb_setup {
	uint8_t		dir_type_recip;
	uint8_t		request;
	uint16_t	value;
	uint16_t	index;
	uint16_t	length;
} __xdata ao_usb_setup;

__pdata uint8_t ao_usb_ep0_state;
uint8_t * __pdata ao_usb_ep0_in_data;
__pdata uint8_t ao_usb_ep0_in_len;
__pdata uint8_t	ao_usb_ep0_in_buf[2];
__pdata uint8_t ao_usb_ep0_out_len;
__xdata uint8_t *__pdata ao_usb_ep0_out_data;
__pdata uint8_t ao_usb_configuration;

/* Send an IN data packet */
static void
ao_usb_ep0_flush(void)
{
	__pdata uint8_t this_len;
	__pdata uint8_t	cs0;

	/* If the IN packet hasn't been picked up, just return */
	USBINDEX = 0;
	cs0 = USBCS0;
	if (cs0 & USBCS0_INPKT_RDY)
		return;

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

__xdata static struct ao_usb_line_coding ao_usb_line_coding = {115200, 0, 0, 8};

/* Walk through the list of descriptors and find a match
 */
static void
ao_usb_get_descriptor(uint16_t value)
{
	__code uint8_t		*__pdata descriptor;
	__pdata uint8_t		type = value >> 8;
	__pdata uint8_t		index = value;

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
	__pdata uint8_t	len;

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
	ao_usb_running = 1;
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
	__pdata uint8_t	cs0;

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

/* Wait for a free IN buffer */
static void
ao_usb_in_wait(void)
{
	for (;;) {
		USBINDEX = AO_USB_IN_EP;
		if ((USBCSIL & USBCSIL_INPKT_RDY) == 0)
			break;
		ao_sleep(&ao_usb_in_bytes);
	}
}

/* Send the current IN packet */
static void
ao_usb_in_send(void)
{
	USBINDEX = AO_USB_IN_EP;
	USBCSIL |= USBCSIL_INPKT_RDY;
	ao_usb_in_bytes_last = ao_usb_in_bytes;
	ao_usb_in_bytes = 0;
}

void
ao_usb_flush(void) __critical
{
	if (!ao_usb_running)
		return;

	/* If there are pending bytes, or if the last packet was full,
	 * send another IN packet
	 */
	if (ao_usb_in_bytes || (ao_usb_in_bytes_last == AO_USB_IN_SIZE)) {
		ao_usb_in_wait();
		ao_usb_in_send();
	}
}

void
ao_usb_putchar(char c) __critical __reentrant
{
	if (!ao_usb_running)
		return;

	ao_usb_in_wait();

	/* Queue a byte, sending the packet when full */
	USBFIFO[AO_USB_IN_EP << 1] = c;
	if (++ao_usb_in_bytes == AO_USB_IN_SIZE)
		ao_usb_in_send();
}

char
ao_usb_pollchar(void) __critical
{
	char c;
	if (ao_usb_out_bytes == 0) {
		USBINDEX = AO_USB_OUT_EP;
		if ((USBCSOL & USBCSOL_OUTPKT_RDY) == 0)
			return AO_READ_AGAIN;
		ao_usb_out_bytes = (USBCNTH << 8) | USBCNTL;
		if (ao_usb_out_bytes == 0) {
			USBINDEX = AO_USB_OUT_EP;
			USBCSOL &= ~USBCSOL_OUTPKT_RDY;
			return AO_READ_AGAIN;
		}
	}
	--ao_usb_out_bytes;
	c = USBFIFO[AO_USB_OUT_EP << 1];
	if (ao_usb_out_bytes == 0) {
		USBINDEX = AO_USB_OUT_EP;
		USBCSOL &= ~USBCSOL_OUTPKT_RDY;
	}
	return c;
}

char
ao_usb_getchar(void) __critical
{
	char	c;

	while ((c = ao_usb_pollchar()) == AO_READ_AGAIN)
		ao_sleep(&ao_stdin_ready);
	return c;
}

void
ao_usb_enable(void)
{
	/* Turn on the USB controller */
	SLEEP |= SLEEP_USB_EN;

	ao_usb_set_configuration();

	ao_usb_set_interrupts();

	/* enable USB interrupts */
	IEN2 |= IEN2_USBIE;

	/* Clear any pending interrupts */
	USBCIF = 0;
	USBOIF = 0;
	USBIIF = 0;
}

void
ao_usb_disable(void)
{
	/* Disable USB interrupts */
	USBIIE = 0;
	USBOIE = 0;
	USBCIE = 0;
	IEN2 &= ~IEN2_USBIE;

	/* Clear any pending interrupts */
	USBCIF = 0;
	USBOIF = 0;
	USBIIF = 0;

	/* Turn off the USB controller */
	SLEEP &= ~SLEEP_USB_EN;
}

void
ao_usb_init(void)
{
	ao_usb_enable();

	ao_add_task(&ao_usb_task, ao_usb_ep0, "usb");
	ao_add_stdio(ao_usb_pollchar, ao_usb_putchar, ao_usb_flush);
}

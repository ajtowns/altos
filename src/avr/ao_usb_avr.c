/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

#define USB_DEBUG 0

#if USB_DEBUG
#define debug(format, args...)	printf(format, ## args)
#else
#define debug(format, args...)
#endif

struct ao_task __xdata ao_usb_task;

struct ao_usb_setup {
	uint8_t		dir_type_recip;
	uint8_t		request;
	uint16_t	value;
	uint16_t	index;
	uint16_t	length;
} __xdata ao_usb_setup;

static __xdata uint8_t 	ao_usb_ep0_state;
static const uint8_t * __xdata ao_usb_ep0_in_data;
static __xdata uint8_t 	ao_usb_ep0_in_len;
static __xdata uint8_t	ao_usb_ep0_in_pending;
static __xdata uint8_t	ao_usb_addr_pending;
static __xdata uint8_t	ao_usb_ep0_in_buf[2];
static __xdata uint8_t 	ao_usb_ep0_out_len;
static __xdata uint8_t *__xdata ao_usb_ep0_out_data;

static __xdata uint8_t	ao_usb_in_flushed;
__xdata uint8_t		ao_usb_running;
static __xdata uint8_t	ao_usb_configuration;
static __xdata uint8_t	ueienx_0;

void
ao_usb_set_address(uint8_t address)
{
	UDADDR = (0 << ADDEN) | address;
	ao_usb_addr_pending = 1;
}

#define EP_SIZE(s)	((s) == 64 ? 0x30 :	\
			((s) == 32 ? 0x20 :	\
			((s) == 16 ? 0x10 :	\
			             0x00)))

static void
ao_usb_dump_ep(uint8_t ep)
{
	UENUM = ep;
	debug ("EP %d: UECONX %02x UECFG0X %02x UECFG1X %02x UEIENX %02x UESTA0X %02x UESTA1X %02X\n",
		ep, UECONX, UECFG0X, UECFG1X, UEIENX, UESTA0X, UESTA1X);
}

static void
ao_usb_set_ep0(void)
{
	debug ("set_ep0\n");
	/* Set the CONTROL max packet size, single buffered */
	UENUM = 0;
	UECONX = (1 << EPEN);					/* Enable */

	UECFG0X = ((0 << EPTYPE0) |				/* Control */
		   (0 << EPDIR));				/* Out (ish) */

	UECFG1X = (EP_SIZE(AO_USB_CONTROL_SIZE) |		/* Size */
		   (0 << EPBK0) |				/* Single bank */
		   (1 << ALLOC));

	ueienx_0 = ((1 << RXSTPE) |				/* Enable SETUP interrupt */
		    (1 << RXOUTE));				/* Enable OUT interrupt */

//	ao_usb_dump_ep(0);
	ao_usb_addr_pending = 0;
}

static void
ao_usb_set_configuration(void)
{
	/* Set the IN max packet size, double buffered */
	UENUM = AO_USB_IN_EP;
	UECONX = (1 << EPEN);					/* Enable */

	UECFG0X = ((2 << EPTYPE0) |				/* Bulk */
		   (1 << EPDIR));				/* In */

	UECFG1X = (EP_SIZE(AO_USB_IN_SIZE) |			/* Size */
		   (1 << EPBK0) |				/* Double bank */
		   (1 << ALLOC));				/* Allocate */

#if 0
	UEIENX = ((1 << TXINE));				/* Enable IN complete interrupt */
#endif

	ao_usb_dump_ep(AO_USB_IN_EP);

	/* Set the OUT max packet size, double buffered */
	UENUM = AO_USB_OUT_EP;
	UECONX |= (1 << EPEN);					/* Enable */

	UECFG0X = ((2 << EPTYPE0) |				/* Bulk */
		   (0 << EPDIR));				/* Out */

	UECFG1X = (EP_SIZE(AO_USB_OUT_SIZE) |			/* Size */
		   (1 << EPBK0) |				/* Double bank */
		   (1 << ALLOC));				/* Allocate */

	UEIENX = ((1 << RXOUTE));				/* Enable OUT complete interrupt */

	ao_usb_dump_ep(AO_USB_OUT_EP);
	ao_usb_running = 1;
}

ISR(USB_GEN_vect)
{
	ao_wakeup(&ao_usb_task);
}


__xdata static struct ao_usb_line_coding ao_usb_line_coding = {115200, 0, 0, 8};

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

static void
ao_usb_ep0_set_in_pending(uint8_t in_pending)
{
	ao_usb_ep0_in_pending = in_pending;

	if (in_pending)
		ueienx_0 = ((1 << RXSTPE) | (1 << RXOUTE) | (1 << TXINE));	/* Enable IN interrupt */
}

/* Send an IN data packet */
static void
ao_usb_ep0_flush(void)
{
	__xdata uint8_t this_len;

	cli();
	UENUM = 0;
	if (!(UEINTX & (1 << TXINI))) {
		debug("EP0 not accepting IN data\n");
		ao_usb_ep0_set_in_pending(1);
	} else {
		this_len = ao_usb_ep0_in_len;
		if (this_len > AO_USB_CONTROL_SIZE)
			this_len = AO_USB_CONTROL_SIZE;

		ao_usb_ep0_in_len -= this_len;

		/* Set IN interrupt enable */
		if (ao_usb_ep0_in_len == 0 && this_len != AO_USB_CONTROL_SIZE)
			ao_usb_ep0_set_in_pending(0);
		else
			ao_usb_ep0_set_in_pending(1);

		debug ("Flush EP0 len %d:", this_len);
		while (this_len--) {
			uint8_t	c = *ao_usb_ep0_in_data++;
			debug(" %02x", c);
			UEDATX = c;
		}
		debug ("\n");

		/* Clear the TXINI bit to send the packet */
		UEINTX &= ~(1 << TXINI);
	}
	sei();
}

/* Read data from the ep0 OUT fifo */
static void
ao_usb_ep0_fill(uint8_t len, uint8_t ack)
{
	if (len > ao_usb_ep0_out_len)
		len = ao_usb_ep0_out_len;
	ao_usb_ep0_out_len -= len;

//	debug ("EP0 UEINTX %02x UEBCLX %d UEBCHX %d\n",
//		UEINTX, UEBCLX, UEBCHX);
	/* Pull all of the data out of the packet */
	debug ("Fill EP0 len %d:", len);
	UENUM = 0;
	while (len--) {
		uint8_t	c = UEDATX;
		*ao_usb_ep0_out_data++ = c;
		debug (" %02x", c);
	}
	debug ("\n");

	/* ACK the packet */
	UEINTX &= ~ack;
}

void
ao_usb_ep0_queue_byte(uint8_t a)
{
	ao_usb_ep0_in_buf[ao_usb_ep0_in_len++] = a;
}

static void
ao_usb_ep0_setup(void)
{
	/* Pull the setup packet out of the fifo */
	ao_usb_ep0_out_data = (__xdata uint8_t *) &ao_usb_setup;
	ao_usb_ep0_out_len = 8;
	ao_usb_ep0_fill(8, (1 << RXSTPI) | (1 << RXOUTI) | (1 << TXINI));
	if (ao_usb_ep0_out_len != 0) {
		debug ("invalid setup packet length\n");
		return;
	}

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
/*
	UENUM = 0;
	if (ao_usb_ep0_state == AO_USB_EP0_IDLE)
		USBCS0 = USBCS0_CLR_OUTPKT_RDY | USBCS0_DATA_END;
	else
		USBCS0 = USBCS0_CLR_OUTPKT_RDY;
*/

	ao_usb_ep0_in_data = ao_usb_ep0_in_buf;
	ao_usb_ep0_in_len = 0;
	switch(ao_usb_setup.dir_type_recip & AO_USB_SETUP_TYPE_MASK) {
	case AO_USB_TYPE_STANDARD:
		debug ("Standard setup packet\n");
		switch(ao_usb_setup.dir_type_recip & AO_USB_SETUP_RECIP_MASK) {
		case AO_USB_RECIP_DEVICE:
			debug ("Device setup packet\n");
			switch(ao_usb_setup.request) {
			case AO_USB_REQ_GET_STATUS:
				debug ("get status\n");
				ao_usb_ep0_queue_byte(0);
				ao_usb_ep0_queue_byte(0);
				break;
			case AO_USB_REQ_SET_ADDRESS:
				debug ("set address %d\n", ao_usb_setup.value);
				ao_usb_set_address(ao_usb_setup.value);
				break;
			case AO_USB_REQ_GET_DESCRIPTOR:
				debug ("get descriptor %d\n", ao_usb_setup.value);
				ao_usb_get_descriptor(ao_usb_setup.value);
				break;
			case AO_USB_REQ_GET_CONFIGURATION:
				debug ("get configuration %d\n", ao_usb_configuration);
				ao_usb_ep0_queue_byte(ao_usb_configuration);
				break;
			case AO_USB_REQ_SET_CONFIGURATION:
				ao_usb_configuration = ao_usb_setup.value;
				debug ("set configuration %d\n", ao_usb_configuration);
				ao_usb_set_configuration();
				break;
			}
			break;
		case AO_USB_RECIP_INTERFACE:
			debug ("Interface setup packet\n");
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
			debug ("Endpoint setup packet\n");
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
		debug ("Class setup packet\n");
		switch (ao_usb_setup.request) {
		case AO_USB_SET_LINE_CODING:
			debug ("set line coding\n");
			ao_usb_ep0_out_len = 7;
			ao_usb_ep0_out_data = (__xdata uint8_t *) &ao_usb_line_coding;
			break;
		case AO_USB_GET_LINE_CODING:
			debug ("get line coding\n");
			ao_usb_ep0_in_len = 7;
			ao_usb_ep0_in_data = (uint8_t *) &ao_usb_line_coding;
			break;
		case AO_USB_SET_CONTROL_LINE_STATE:
			break;
		}
		break;
	}
	if (ao_usb_ep0_state != AO_USB_EP0_DATA_OUT) {
		if (ao_usb_setup.length < ao_usb_ep0_in_len)
			ao_usb_ep0_in_len = ao_usb_setup.length;
		debug ("Start ep0 in delivery %d\n", ao_usb_ep0_in_len);
		ao_usb_ep0_set_in_pending(1);
	}
}

/* End point 0 receives all of the control messages. */
static void
ao_usb_ep0(void)
{
	uint8_t	intx, udint;

	debug ("usb task started\n");
	ao_usb_ep0_state = AO_USB_EP0_IDLE;
	for (;;) {
		cli();
		for (;;) {
			udint = UDINT;
			UDINT = 0;
//			debug ("UDINT %02x\n", udint);
			if (udint & (1 << EORSTI)) {
				ao_usb_configuration = 0;
				ao_usb_set_ep0();
			}
			UENUM = 0;
			intx = UEINTX;
//			debug ("UEINTX %02x\n", intx);
			if (intx & ((1 << RXSTPI) | (1 << RXOUTI)))
				break;
			if ((intx & (1 << TXINI))) {
				if (ao_usb_ep0_in_pending)
					break;
				else
				{
					if (ao_usb_addr_pending) {
						UDADDR |= (1 << ADDEN);
						ao_usb_addr_pending = 0;
					}
					ueienx_0 = ((1 << RXSTPE) | (1 << RXOUTE));	/* Disable IN interrupt */
				}
			}
//			debug ("usb task sleeping...\n");
			UENUM = 0;
			UEIENX = ueienx_0;
			ao_sleep(&ao_usb_task);
		}
		sei();
//		debug ("UEINTX for ep0 is %02x\n", intx);
		if (intx & (1 << RXSTPI)) {
			ao_usb_ep0_setup();
		}
		if (intx & (1 << RXOUTI)) {
			ao_usb_ep0_fill(UEBCLX, (1 << RXOUTI));
			ao_usb_ep0_set_in_pending(1);
		}
		if (intx & (1 << TXINI) && ao_usb_ep0_in_pending) {
			debug ("continue sending ep0 IN data\n");
			ao_usb_ep0_flush();
		}
	}
}

/* Wait for a free IN buffer */
static void
_ao_usb_in_wait(void)
{
	for (;;) {
		/* Check if the current buffer is writable */
		UENUM = AO_USB_IN_EP;
		if (UEINTX & (1 << RWAL))
			break;

		/* Wait for an IN buffer to be ready */
		for (;;) {
			UENUM = AO_USB_IN_EP;
			if ((UEINTX & (1 << TXINI)))
				break;
			UEIENX = (1 << TXINE);
			ao_sleep(&ao_usb_in_flushed);
		}
		/* Ack the interrupt */
		UEINTX &= ~(1 << TXINI);
	}
}

/* Queue the current IN buffer for transmission */
static void
_ao_usb_in_send(void)
{
	UENUM = AO_USB_IN_EP;
	UEINTX &= ~(1 << FIFOCON);
}

void
ao_usb_flush(void)
{
	if (!ao_usb_running)
		return;

	ao_arch_block_interrupts();
	/* Anytime we've sent a character since
	 * the last time we flushed, we'll need
	 * to send a packet -- the only other time
	 * we would send a packet is when that
	 * packet was full, in which case we now
	 * want to send an empty packet
	 */
	if (!ao_usb_in_flushed) {
		ao_usb_in_flushed = 1;
		_ao_usb_in_wait();
		_ao_usb_in_send();
	}
	ao_arch_release_interrupts();
}

void
ao_usb_putchar(char c)
{
	if (!ao_usb_running)
		return;

	ao_arch_block_interrupts();
	_ao_usb_in_wait();

	/* Queue a byte */
	UENUM = AO_USB_IN_EP;
	UEDATX = c;

	/* Send the packet when full */
	if ((UEINTX & (1 << RWAL)) == 0)
		_ao_usb_in_send();
	ao_usb_in_flushed = 0;
	ao_arch_release_interrupts();
}

int
_ao_usb_pollchar(void)
{
	uint8_t c;
	uint8_t	intx;

	if (!ao_usb_running)
		return AO_READ_AGAIN;

	for (;;) {
		UENUM = AO_USB_OUT_EP;
		intx = UEINTX;
		debug("usb_pollchar UEINTX %02d\n", intx);
		if (intx & (1 << RWAL))
			break;

		if (intx & (1 << FIFOCON)) {
			/* Ack the last packet */
			UEINTX = (uint8_t) ~(1 << FIFOCON);
		}

		/* Check to see if a packet has arrived */
		if ((intx & (1 << RXOUTI)) == 0) {
			UENUM = AO_USB_OUT_EP;
			UEIENX = (1 << RXOUTE);
			return AO_READ_AGAIN;
		}

		/* Ack the interrupt */
		UEINTX = ~(1 << RXOUTI);
	}

	/* Pull a character out of the fifo */
	c = UEDATX;
	return c;
}

char
ao_usb_getchar(void)
{
	int	c;

	ao_arch_block_interrupts();
	while ((c = _ao_usb_pollchar()) == AO_READ_AGAIN)
		ao_sleep(&ao_stdin_ready);
	ao_arch_release_interrupts();
	return c;
}

uint16_t	control_count;
uint16_t	in_count;
uint16_t	out_count;

/* Endpoint interrupt */
ISR(USB_COM_vect)
{
	uint8_t	old_num = UENUM;
	uint8_t	i = UEINT;

#ifdef AO_LED_RED
	ao_led_toggle(AO_LED_RED);
#endif
	UEINT = 0;
	if (i & (1 << 0)) {
		UENUM = 0;
		UEIENX = 0;
		ao_wakeup(&ao_usb_task);
		++control_count;
	}
	if (i & (1 << AO_USB_IN_EP)) {
		UENUM = AO_USB_IN_EP;
		UEIENX = 0;
		ao_wakeup(&ao_usb_in_flushed);
		in_count++;
	}
	if (i & (1 << AO_USB_OUT_EP)) {
		UENUM = AO_USB_OUT_EP;
		UEIENX = 0;
		ao_wakeup(&ao_stdin_ready);
		++out_count;
	}
	UENUM = old_num;
}

#if AVR_VCC_5V
#define AO_PAD_REGULATOR_INIT	(1 << UVREGE)	/* Turn on pad regulator */
#endif
#if AVR_VCC_3V3
/* TeleScience V0.1 has a hardware bug -- UVcc is hooked up, but UCap is not
 * Make this work by running power through UVcc to the USB system
 */
#define AO_PAD_REGULATOR_INIT	(1 << UVREGE)	/* Turn off pad regulator */
#endif

#if AVR_CLOCK == 16000000UL
#define AO_USB_PLL_INPUT_PRESCALER	(1 << PINDIV)	/* Divide 16MHz clock by 2 */
#endif
#if AVR_CLOCK == 8000000UL
#define AO_USB_PLL_INPUT_PRESCALER	0		/* Don't divide clock */
#endif

void
ao_usb_disable(void)
{
	/* Unplug from the bus */
	UDCON = (1 << DETACH);

	/* Disable the interface */
	USBCON = 0;

	/* Disable the PLL */
	PLLCSR = 0;

	/* Turn off the pad regulator */
	UHWCON = 0;
}

#define AO_USB_CON ((1 << USBE) |	/* USB enable */ \
		    (0 << RSTCPU) |	/* do not reset CPU */	\
		    (0 << LSM) |	/* Full speed mode */	\
		    (0 << RMWKUP))	/* no remote wake-up */ \

void
ao_usb_enable(void)
{
	/* Configure pad regulator */
	UHWCON = AO_PAD_REGULATOR_INIT;

	/* Enable USB device, but freeze the clocks until initialized */
	USBCON = AO_USB_CON | (1 <<FRZCLK);

	/* Enable PLL with appropriate divider */
	PLLCSR = AO_USB_PLL_INPUT_PRESCALER | (1 << PLLE);

	/* Wait for PLL to lock */
	loop_until_bit_is_set(PLLCSR, (1 << PLOCK));

	/* Enable USB, enable the VBUS pad */
	USBCON = AO_USB_CON | (1 << OTGPADE);

	/* Enable global interrupts */
	UDIEN = (1 << EORSTE);		/* End of reset interrupt */

	ao_usb_configuration = 0;

	debug ("ao_usb_enable\n");

	debug ("UHWCON %02x USBCON %02x PLLCSR %02x UDIEN %02x\n",
	       UHWCON, USBCON, PLLCSR, UDIEN);
	UDCON = (0 << DETACH);	/* Clear the DETACH bit to plug into the bus */
}

#if USB_DEBUG
struct ao_task __xdata ao_usb_echo_task;

static void
ao_usb_echo(void)
{
	char	c;

	for (;;) {
		c = ao_usb_getchar();
		ao_usb_putchar(c);
		ao_usb_flush();
	}
}
#endif

void
ao_usb_init(void)
{
	ao_usb_enable();

	debug ("ao_usb_init\n");
	ao_add_task(&ao_usb_task, ao_usb_ep0, "usb");
#if USB_DEBUG
	ao_add_task(&ao_usb_echo_task, ao_usb_echo, "usb echo");
#endif
	ao_add_stdio(_ao_usb_pollchar, ao_usb_putchar, ao_usb_flush);
}

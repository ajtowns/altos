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

#include "ao.h"
#include "ao_usb.h"
#include "ao_product.h"

#ifndef USE_USB_STDIO
#define USE_USB_STDIO	1
#endif

#if USE_USB_STDIO
#define AO_USB_OUT_SLEEP_ADDR	(&ao_stdin_ready)
#else
#define AO_USB_OUT_SLEEP_ADDR	(&ao_usb_out_avail)
#endif

#define USB_DEBUG 	0
#define USB_DEBUG_DATA	0
#define USB_ECHO	0

#if USB_DEBUG
#define debug(format, args...)	printf(format, ## args);
#else
#define debug(format, args...)
#endif

#if USB_DEBUG_DATA
#define debug_data(format, args...)	printf(format, ## args);
#else
#define debug_data(format, args...)
#endif

struct ao_usb_setup {
	uint8_t		dir_type_recip;
	uint8_t		request;
	uint16_t	value;
	uint16_t	index;
	uint16_t	length;
} ao_usb_setup;

static uint8_t 	ao_usb_ep0_state;

/* Pending EP0 IN data */
static const uint8_t	*ao_usb_ep0_in_data;	/* Remaining data */
static uint8_t 		ao_usb_ep0_in_len;	/* Remaining amount */
static uint16_t		ao_usb_ep0_in_max;	/* Requested amount from host */

/* Temp buffer for smaller EP0 in data */
static uint8_t	ao_usb_ep0_in_buf[2];

/* Pending EP0 OUT data */
static uint8_t *ao_usb_ep0_out_data;
static uint8_t 	ao_usb_ep0_out_len;

/*
 * Objects allocated in special USB memory
 */

/* USB address of end of allocated storage */
static uint8_t	*ao_usb_sram;

/* Pointer to ep0 tx/rx buffers in USB memory */
static uint8_t	*ao_usb_ep0_tx_buffer;
static uint8_t	*ao_usb_ep0_setup_buffer;
static uint8_t	*ao_usb_ep0_rx_buffer;

/* Pointer to bulk data tx/rx buffers in USB memory */
static uint8_t	*ao_usb_in_tx_buffer;
static uint8_t	*ao_usb_out_rx_buffer;

/* Our data buffers */
static uint8_t	ao_usb_tx_buffer[AO_USB_IN_SIZE];
static uint8_t	ao_usb_tx_count;

static uint8_t	ao_usb_rx_buffer[AO_USB_OUT_SIZE];
static uint8_t	ao_usb_rx_count, ao_usb_rx_pos;

extern struct lpc_usb_endpoint lpc_usb_endpoint;

/* Marks when we don't need to send an IN packet.
 * This happens only when the last IN packet is not full,
 * otherwise the host will expect to keep seeing packets.
 * Send a zero-length packet as required
 */
static uint8_t	ao_usb_in_flushed;

/* Marks when we have delivered an IN packet to the hardware
 * and it has not been received yet. ao_sleep on this address
 * to wait for it to be delivered.
 */
static uint8_t	ao_usb_in_pending;

/* Marks when an OUT packet has been received by the hardware
 * but not pulled to the shadow buffer.
 */
static uint8_t	ao_usb_out_avail;
uint8_t		ao_usb_running;
static uint8_t	ao_usb_configuration;

#define AO_USB_EP0_GOT_RESET	1
#define AO_USB_EP0_GOT_SETUP	2
#define AO_USB_EP0_GOT_RX_DATA	4
#define AO_USB_EP0_GOT_TX_ACK	8

static uint8_t	ao_usb_ep0_receive;
static uint8_t	ao_usb_address;
static uint8_t	ao_usb_address_pending;

static inline uint32_t set_toggle(uint32_t 	current_value,
				   uint32_t	mask,
				   uint32_t	desired_value)
{
	return (current_value ^ desired_value) & mask;
}

/*
 * Set current device address and mark the
 * interface as active
 */
void
ao_usb_set_address(uint8_t address)
{
	debug("ao_usb_set_address %02x\n", address);
	lpc_usb.devcmdstat = ((address << LPC_USB_DEVCMDSTAT_DEV_ADDR) |
			      (1 << LPC_USB_DEVCMDSTAT_DEV_EN) |
			      (0 << LPC_USB_DEVCMDSTAT_SETUP) |
			      (0 << LPC_USB_DEVCMDSTAT_PLL_ON) |
			      (0 << LPC_USB_DEVCMDSTAT_LPM_SUP) |
			      (0 << LPC_USB_DEVCMDSTAT_INTONNAK_AO) |
			      (0 << LPC_USB_DEVCMDSTAT_INTONNAK_AI) |
			      (0 << LPC_USB_DEVCMDSTAT_INTONNAK_CO) |
			      (0 << LPC_USB_DEVCMDSTAT_INTONNAK_CI) |
			      (1 << LPC_USB_DEVCMDSTAT_DCON) |
			      (0 << LPC_USB_DEVCMDSTAT_DSUS) |
			      (0 << LPC_USB_DEVCMDSTAT_DCON_C) |
			      (0 << LPC_USB_DEVCMDSTAT_DSUS_C) |
			      (0 << LPC_USB_DEVCMDSTAT_DRES_C) |
			      (0 << LPC_USB_DEVCMDSTAT_VBUSDEBOUNCED));
	ao_usb_address_pending = 0;
}

#define TX_DBG 0
#define RX_DBG 0

#if TX_DBG
#define _tx_dbg0(msg) _dbg(__LINE__,msg,0)
#define _tx_dbg1(msg,value) _dbg(__LINE__,msg,value)
#else
#define _tx_dbg0(msg)
#define _tx_dbg1(msg,value)
#endif

#if RX_DBG
#define _rx_dbg0(msg) _dbg(__LINE__,msg,0)
#define _rx_dbg1(msg,value) _dbg(__LINE__,msg,value)
#else
#define _rx_dbg0(msg)
#define _rx_dbg1(msg,value)
#endif

#if TX_DBG || RX_DBG
static void _dbg(int line, char *msg, uint32_t value);
#endif

/*
 * Set just endpoint 0, for use during startup
 */

static uint8_t *
ao_usb_alloc_sram(uint16_t size)
{
	uint8_t	*addr = ao_usb_sram;

	ao_usb_sram += (size + 63) & ~63;
	return addr;
}

static uint16_t
ao_usb_sram_offset(uint8_t *addr)
{
	return (uint16_t) ((intptr_t) addr >> 6);
}

static void
ao_usb_set_ep(vuint32_t *ep, uint8_t *addr, uint16_t nbytes)
{
	*ep = ((ao_usb_sram_offset(addr) << LPC_USB_EP_OFFSET) |
	       (nbytes << LPC_USB_EP_NBYTES) |
	       (0 << LPC_USB_EP_ENDPOINT_ISO) |
	       (0 << LPC_USB_EP_RATE_FEEDBACK) |
	       (0 << LPC_USB_EP_TOGGLE_RESET) |
	       (0 << LPC_USB_EP_STALL) |
	       (0 << LPC_USB_EP_DISABLED) |
	       (1 << LPC_USB_EP_ACTIVE));
}

static inline uint16_t
ao_usb_ep_count(vuint32_t *ep)
{
	return (*ep >> LPC_USB_EP_NBYTES) & LPC_USB_EP_NBYTES_MASK;
}

static inline uint8_t
ao_usb_ep_stall(vuint32_t *ep)
{
	return (*ep >> LPC_USB_EP_STALL) & 1;
}

static inline vuint32_t *
ao_usb_ep0_out(void)
{
	return &lpc_usb_endpoint.ep0_out;
}

static inline vuint32_t *
ao_usb_ep0_in(void)
{
	return &lpc_usb_endpoint.ep0_in;
}

static inline vuint32_t *
ao_usb_epn_out(uint8_t n)
{
	return &lpc_usb_endpoint.epn[n-1].out[0];
}

static inline vuint32_t *
ao_usb_epn_in(uint8_t n)
{
	return &lpc_usb_endpoint.epn[n-1].in[0];
}

#if UNUSED
static void
ao_usb_set_epn_in(uint8_t n, uint8_t *addr, uint16_t nbytes)
{
	ao_usb_set_ep(ao_usb_epn_in(n), addr, nbytes);
}
#endif

static void
ao_usb_set_epn_out(uint8_t n, uint8_t *addr, uint16_t nbytes)
{
	ao_usb_set_ep(ao_usb_epn_out(n), addr, nbytes);
}

static inline uint16_t
ao_usb_epn_out_count(uint8_t n)
{
	return ao_usb_ep_count(ao_usb_epn_out(n));
}

static inline uint16_t
ao_usb_epn_in_count(uint8_t n)
{
	return ao_usb_ep_count(ao_usb_epn_in(n));
}

static uint8_t *
ao_usb_enable_ep(vuint32_t *ep, uint16_t nbytes, uint16_t set_nbytes)
{
	uint8_t	*addr = ao_usb_alloc_sram(nbytes);
	
	ao_usb_set_ep(ep, addr, set_nbytes);
	return addr;
}

static void
ao_usb_disable_ep(vuint32_t *ep)
{
	*ep = ((0 << LPC_USB_EP_OFFSET) |
	       (0 << LPC_USB_EP_NBYTES) |
	       (0 << LPC_USB_EP_ENDPOINT_ISO) |
	       (0 << LPC_USB_EP_RATE_FEEDBACK) |
	       (0 << LPC_USB_EP_TOGGLE_RESET) |
	       (0 << LPC_USB_EP_STALL) |
	       (1 << LPC_USB_EP_DISABLED) |
	       (0 << LPC_USB_EP_ACTIVE));
}

static void
ao_usb_enable_epn(uint8_t n, uint16_t out_bytes, uint8_t **out_addr, uint16_t in_bytes, uint8_t **in_addr)
{
	uint8_t	*addr;

	addr = ao_usb_enable_ep(ao_usb_epn_out(n), out_bytes, out_bytes);
	if (out_addr)
		*out_addr = addr;
	ao_usb_disable_ep(&lpc_usb_endpoint.epn[n-1].out[1]);

	addr = ao_usb_enable_ep(ao_usb_epn_in(n), in_bytes, 0);
	if (in_addr)
		*in_addr = addr;
	ao_usb_disable_ep(&lpc_usb_endpoint.epn[n-1].in[1]);
}

static void
ao_usb_disable_epn(uint8_t n)
{
	ao_usb_disable_ep(ao_usb_epn_out(n));
	ao_usb_disable_ep(&lpc_usb_endpoint.epn[n-1].out[1]);
	ao_usb_disable_ep(ao_usb_epn_in(n));
	ao_usb_disable_ep(&lpc_usb_endpoint.epn[n-1].in[1]);
}

static void
ao_usb_reset(void)
{
	ao_usb_set_address(0);
	ao_usb_configuration = 0;
}

static void
ao_usb_set_ep0(void)
{
	int			e;

	/* Everything is single buffered for now */
	lpc_usb.epbufcfg = 0;
	lpc_usb.epinuse = 0;
	lpc_usb.epskip = 0xffffffff;

	lpc_usb.intstat = 0xc00003ff;

	ao_usb_sram = lpc_usb_sram;

	lpc_usb.epliststart = (uint32_t) (intptr_t) &lpc_usb_endpoint;
	lpc_usb.databufstart = ((uint32_t) (intptr_t) ao_usb_sram) & 0xffc00000;

	/* Set up EP 0 - a Control end point with 32 bytes of in and out buffers */

	ao_usb_ep0_rx_buffer = ao_usb_enable_ep(ao_usb_ep0_out(), AO_USB_CONTROL_SIZE, AO_USB_CONTROL_SIZE);
	ao_usb_ep0_setup_buffer = ao_usb_alloc_sram(AO_USB_CONTROL_SIZE);
	lpc_usb_endpoint.setup = ao_usb_sram_offset(ao_usb_ep0_setup_buffer);
	ao_usb_ep0_tx_buffer = ao_usb_enable_ep(ao_usb_ep0_in(), AO_USB_CONTROL_SIZE, 0);

	/* Clear all of the other endpoints */
	for (e = 1; e <= 4; e++)
		ao_usb_disable_epn(e);
	ao_usb_reset();
}

static void
ao_usb_set_configuration(void)
{
	debug ("ao_usb_set_configuration\n");

	/* Set up the INT end point */
	ao_usb_enable_epn(AO_USB_INT_EP, 0, NULL, 0, NULL);
	
	/* Set up the OUT end point */
	ao_usb_enable_epn(AO_USB_OUT_EP, AO_USB_OUT_SIZE, &ao_usb_out_rx_buffer, 0, NULL);

	/* Set up the IN end point */
	ao_usb_enable_epn(AO_USB_IN_EP, 0, NULL, AO_USB_IN_SIZE, &ao_usb_in_tx_buffer);

	ao_usb_running = 1;
}

/* Send an IN data packet */
static void
ao_usb_ep0_flush(void)
{
	uint8_t this_len;

	this_len = ao_usb_ep0_in_len;
	if (this_len > AO_USB_CONTROL_SIZE)
		this_len = AO_USB_CONTROL_SIZE;

	ao_usb_ep0_in_len -= this_len;
	ao_usb_ep0_in_max -= this_len;

	if (this_len < AO_USB_CONTROL_SIZE || ao_usb_ep0_in_max == 0)
		ao_usb_ep0_state = AO_USB_EP0_IDLE;

	debug_data ("Flush EP0 len %d:", this_len);
	memcpy(ao_usb_ep0_tx_buffer, ao_usb_ep0_in_data, this_len);
	debug_data ("\n");
	ao_usb_ep0_in_data += this_len;

	/* Mark the endpoint as TX valid to send the packet */
	ao_usb_set_ep(ao_usb_ep0_in(), ao_usb_ep0_tx_buffer, this_len);
	debug ("queue tx.  0 now %08x\n", *ao_usb_ep0_in());
}

/* Read data from the ep0 OUT fifo */
static void
ao_usb_ep0_fill(void)
{
	uint16_t	len;
	uint8_t		*rx_buffer;

	/* Pull all of the data out of the packet */
	if (lpc_usb.devcmdstat & (1 << LPC_USB_DEVCMDSTAT_SETUP)) {
		rx_buffer = ao_usb_ep0_setup_buffer;
		len = 8;
	} else {
		rx_buffer = ao_usb_ep0_rx_buffer;
		len = AO_USB_CONTROL_SIZE - ao_usb_ep_count(ao_usb_ep0_out());
	}

	if (len > ao_usb_ep0_out_len)
		len = ao_usb_ep0_out_len;
	ao_usb_ep0_out_len -= len;

	debug_data ("Fill EP0 len %d:", len);
	memcpy(ao_usb_ep0_out_data, rx_buffer, len);
	debug_data ("\n");
	ao_usb_ep0_out_data += len;

	/* ACK the packet */
	ao_usb_set_ep(ao_usb_ep0_out(), ao_usb_ep0_rx_buffer, AO_USB_CONTROL_SIZE);
	lpc_usb.devcmdstat |= (1 << LPC_USB_DEVCMDSTAT_SETUP);
}

static void
ao_usb_ep0_in_reset(void)
{
	ao_usb_ep0_in_data = ao_usb_ep0_in_buf;
	ao_usb_ep0_in_len = 0;
}

static void
ao_usb_ep0_in_queue_byte(uint8_t a)
{
	if (ao_usb_ep0_in_len < sizeof (ao_usb_ep0_in_buf))
		ao_usb_ep0_in_buf[ao_usb_ep0_in_len++] = a;
}

static void
ao_usb_ep0_in_set(const uint8_t *data, uint8_t len)
{
	ao_usb_ep0_in_data = data;
	ao_usb_ep0_in_len = len;
}

static void
ao_usb_ep0_out_set(uint8_t *data, uint8_t len)
{
	ao_usb_ep0_out_data = data;
	ao_usb_ep0_out_len = len;
}

static void
ao_usb_ep0_in_start(uint16_t max)
{
	ao_usb_ep0_in_max = max;
	/* Don't send more than asked for */
	if (ao_usb_ep0_in_len > max)
		ao_usb_ep0_in_len = max;
	ao_usb_ep0_flush();
}

static struct ao_usb_line_coding ao_usb_line_coding = {115200, 0, 0, 8};

/* Walk through the list of descriptors and find a match
 */
static void
ao_usb_get_descriptor(uint16_t value)
{
	const uint8_t		*descriptor;
	uint8_t		type = value >> 8;
	uint8_t		index = value;

	descriptor = ao_usb_descriptors;
	while (descriptor[0] != 0) {
		if (descriptor[1] == type && index-- == 0) {
			uint8_t	len;
			if (type == AO_USB_DESC_CONFIGURATION)
				len = descriptor[2];
			else
				len = descriptor[0];
			ao_usb_ep0_in_set(descriptor, len);
			break;
		}
		descriptor += descriptor[0];
	}
}

static void
ao_usb_ep0_setup(void)
{
	/* Pull the setup packet out of the fifo */
	ao_usb_ep0_out_set((uint8_t *) &ao_usb_setup, 8);
	ao_usb_ep0_fill();
	if (ao_usb_ep0_out_len != 0) {
		debug ("invalid setup packet length\n");
		return;
	}

	if ((ao_usb_setup.dir_type_recip & AO_USB_DIR_IN) || ao_usb_setup.length == 0)
		ao_usb_ep0_state = AO_USB_EP0_DATA_IN;
	else
		ao_usb_ep0_state = AO_USB_EP0_DATA_OUT;

	ao_usb_ep0_in_reset();

	switch(ao_usb_setup.dir_type_recip & AO_USB_SETUP_TYPE_MASK) {
	case AO_USB_TYPE_STANDARD:
		debug ("Standard setup packet\n");
		switch(ao_usb_setup.dir_type_recip & AO_USB_SETUP_RECIP_MASK) {
		case AO_USB_RECIP_DEVICE:
			debug ("Device setup packet\n");
			switch(ao_usb_setup.request) {
			case AO_USB_REQ_GET_STATUS:
				debug ("get status\n");
				ao_usb_ep0_in_queue_byte(0);
				ao_usb_ep0_in_queue_byte(0);
				break;
			case AO_USB_REQ_SET_ADDRESS:
				debug ("set address %d\n", ao_usb_setup.value);
				ao_usb_address = ao_usb_setup.value;
				ao_usb_address_pending = 1;
				break;
			case AO_USB_REQ_GET_DESCRIPTOR:
				debug ("get descriptor %d\n", ao_usb_setup.value);
				ao_usb_get_descriptor(ao_usb_setup.value);
				break;
			case AO_USB_REQ_GET_CONFIGURATION:
				debug ("get configuration %d\n", ao_usb_configuration);
				ao_usb_ep0_in_queue_byte(ao_usb_configuration);
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
				ao_usb_ep0_in_queue_byte(0);
				ao_usb_ep0_in_queue_byte(0);
				break;
			case AO_USB_REQ_GET_INTERFACE:
				ao_usb_ep0_in_queue_byte(0);
				break;
			case AO_USB_REQ_SET_INTERFACE:
				break;
			}
			break;
		case AO_USB_RECIP_ENDPOINT:
			debug ("Endpoint setup packet\n");
			switch(ao_usb_setup.request) {
			case AO_USB_REQ_GET_STATUS:
				ao_usb_ep0_in_queue_byte(0);
				ao_usb_ep0_in_queue_byte(0);
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
			ao_usb_ep0_out_set((uint8_t *) &ao_usb_line_coding, 7);
			break;
		case AO_USB_GET_LINE_CODING:
			debug ("get line coding\n");
			ao_usb_ep0_in_set((const uint8_t *) &ao_usb_line_coding, 7);
			break;
		case AO_USB_SET_CONTROL_LINE_STATE:
			break;
		}
		break;
	}

	/* If we're not waiting to receive data from the host,
	 * queue an IN response
	 */
	if (ao_usb_ep0_state == AO_USB_EP0_DATA_IN)
		ao_usb_ep0_in_start(ao_usb_setup.length);
}

static void
ao_usb_ep0_handle(uint8_t receive)
{
	ao_usb_ep0_receive = 0;

	if (receive & AO_USB_EP0_GOT_RESET) {
		debug ("\treset\n");
		ao_usb_reset();
		return;
	}
	if (receive & AO_USB_EP0_GOT_SETUP) {
		debug ("\tsetup\n");
		ao_usb_ep0_setup();
	}
	if (receive & AO_USB_EP0_GOT_RX_DATA) {
		debug ("\tgot rx data\n");
		if (ao_usb_ep0_state == AO_USB_EP0_DATA_OUT) {
			ao_usb_ep0_fill();
			if (ao_usb_ep0_out_len == 0) {
				ao_usb_ep0_state = AO_USB_EP0_DATA_IN;
				ao_usb_ep0_in_start(0);
			}
		}
	}
	if (receive & AO_USB_EP0_GOT_TX_ACK) {
		debug ("\tgot tx ack\n");

		/* Wait until the IN packet is received from addr 0
		 * before assigning our local address
		 */
		if (ao_usb_address_pending) {
#if HAS_FLIGHT
			/* Go to idle mode if USB is connected
			 */
			ao_flight_force_idle = 1;
#endif
			ao_usb_set_address(ao_usb_address);
		}
		if (ao_usb_ep0_state == AO_USB_EP0_DATA_IN)
			ao_usb_ep0_flush();
	}
}

#if USB_DEBUG
static uint16_t int_count;
static uint16_t	in_count;
static uint16_t	out_count;
static uint16_t	reset_count;
#endif

void
lpc_usb_irq_isr(void)
{
	uint32_t	intstat = lpc_usb.intstat & lpc_usb.inten;

	lpc_usb.intstat = intstat;
	/* Handle EP0 OUT packets */
	if (intstat & (1 << LPC_USB_INT_EPOUT(0))) {
		if (lpc_usb.devcmdstat & (1 << LPC_USB_DEVCMDSTAT_SETUP))
			ao_usb_ep0_receive |= AO_USB_EP0_GOT_SETUP;
		else
			ao_usb_ep0_receive |= AO_USB_EP0_GOT_RX_DATA;

		ao_usb_ep0_handle(ao_usb_ep0_receive);
	}

	/* Handle EP0 IN packets */
	if (intstat & (1 << LPC_USB_INT_EPIN(0))) {
		ao_usb_ep0_receive |= AO_USB_EP0_GOT_TX_ACK;

		ao_usb_ep0_handle(ao_usb_ep0_receive);
	}


	/* Handle OUT packets */
	if (intstat & (1 << LPC_USB_INT_EPOUT(AO_USB_OUT_EP))) {
#if USB_DEBUG
		++out_count;
#endif
		_rx_dbg1("RX ISR", *ao_usb_epn_out(AO_USB_OUT_EP));
		ao_usb_out_avail = 1;
		_rx_dbg0("out avail set");
		ao_wakeup(AO_USB_OUT_SLEEP_ADDR)
		_rx_dbg0("stdin awoken");
	}

	/* Handle IN packets */
	if (intstat & (1 << LPC_USB_INT_EPIN(AO_USB_IN_EP))) {
#if USB_DEBUG
		++in_count;
#endif
		_tx_dbg1("TX ISR", *ao_usb_epn_in(AO_USB_IN_EP));
		ao_usb_in_pending = 0;
		ao_wakeup(&ao_usb_in_pending);
	}

	/* NAK all INT EP IN packets */
	if (intstat & (1 << LPC_USB_INT_EPIN(AO_USB_INT_EP))) {
		;
	}

	/* Check for reset */
	if (intstat & (1 << LPC_USB_INT_DEV)) {
		if (lpc_usb.devcmdstat & (1 << LPC_USB_DEVCMDSTAT_DRES_C))
		{
			lpc_usb.devcmdstat |= (1 << LPC_USB_DEVCMDSTAT_DRES_C);
			ao_usb_ep0_receive |= AO_USB_EP0_GOT_RESET;
			ao_usb_ep0_handle(ao_usb_ep0_receive);
		}
	}
}



/* Queue the current IN buffer for transmission */
static void
_ao_usb_in_send(void)
{
	_tx_dbg0("in_send start");
	debug ("send %d\n", ao_usb_tx_count);
	while (ao_usb_in_pending)
		ao_sleep(&ao_usb_in_pending);
	ao_usb_in_pending = 1;
	if (ao_usb_tx_count != AO_USB_IN_SIZE)
		ao_usb_in_flushed = 1;
	memcpy(ao_usb_in_tx_buffer, ao_usb_tx_buffer, ao_usb_tx_count);
	ao_usb_set_ep(ao_usb_epn_in(AO_USB_IN_EP), ao_usb_in_tx_buffer, ao_usb_tx_count);
	ao_usb_tx_count = 0;
	_tx_dbg0("in_send end");
}

/* Wait for a free IN buffer. Interrupts are blocked */
static void
_ao_usb_in_wait(void)
{
	for (;;) {
		/* Check if the current buffer is writable */
		if (ao_usb_tx_count < AO_USB_IN_SIZE)
			break;

		_tx_dbg0("in_wait top");
		/* Wait for an IN buffer to be ready */
		while (ao_usb_in_pending)
			ao_sleep(&ao_usb_in_pending);
		_tx_dbg0("in_wait bottom");
	}
}

void
ao_usb_flush(void)
{
	if (!ao_usb_running)
		return;

	/* Anytime we've sent a character since
	 * the last time we flushed, we'll need
	 * to send a packet -- the only other time
	 * we would send a packet is when that
	 * packet was full, in which case we now
	 * want to send an empty packet
	 */
	ao_arch_block_interrupts();
	while (!ao_usb_in_flushed) {
		_tx_dbg0("flush top");
		_ao_usb_in_send();
		_tx_dbg0("flush end");
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

	ao_usb_in_flushed = 0;
	ao_usb_tx_buffer[ao_usb_tx_count++] = (uint8_t) c;

	/* Send the packet when full */
	if (ao_usb_tx_count == AO_USB_IN_SIZE) {
		_tx_dbg0("putchar full");
		_ao_usb_in_send();
		_tx_dbg0("putchar flushed");
	}
	ao_arch_release_interrupts();
}

static void
_ao_usb_out_recv(void)
{
	_rx_dbg0("out_recv top");
	ao_usb_out_avail = 0;

	ao_usb_rx_count = AO_USB_OUT_SIZE - ao_usb_epn_out_count(AO_USB_OUT_EP);

	_rx_dbg1("out_recv count", ao_usb_rx_count);
	debug ("recv %d\n", ao_usb_rx_count);
	debug_data("Fill OUT len %d:", ao_usb_rx_count);
	memcpy(ao_usb_rx_buffer, ao_usb_out_rx_buffer, ao_usb_rx_count);
	debug_data("\n");
	ao_usb_rx_pos = 0;

	/* ACK the packet */
	ao_usb_set_epn_out(AO_USB_OUT_EP, ao_usb_out_rx_buffer, AO_USB_OUT_SIZE);
}

int
_ao_usb_pollchar(void)
{
	uint8_t c;

	if (!ao_usb_running)
		return AO_READ_AGAIN;

	for (;;) {
		if (ao_usb_rx_pos != ao_usb_rx_count)
			break;

		_rx_dbg0("poll check");
		/* Check to see if a packet has arrived */
		if (!ao_usb_out_avail) {
			_rx_dbg0("poll none");
			return AO_READ_AGAIN;
		}
		_ao_usb_out_recv();
	}

	/* Pull a character out of the fifo */
	c = ao_usb_rx_buffer[ao_usb_rx_pos++];
	return c;
}

char
ao_usb_getchar(void)
{
	int	c;

	ao_arch_block_interrupts();
	while ((c = _ao_usb_pollchar()) == AO_READ_AGAIN)
		ao_sleep(AO_USB_OUT_SLEEP_ADDR);
	ao_arch_release_interrupts();
	return c;
}

void
ao_usb_disable(void)
{
	ao_arch_block_interrupts();

#if HAS_USB_PULLUP
	ao_gpio_set(AO_USB_PULLUP_PORT, AO_USB_PULLUP_PIN, AO_USB_PULLUP, 0);
#endif
	/* Disable interrupts */
	lpc_usb.inten = 0;

	lpc_nvic_clear_enable(LPC_ISR_USB_IRQ_POS);

	/* Disable the device */
	lpc_usb.devcmdstat = 0;

	/* Turn off USB clock */
	lpc_scb.usbclkdiv = 0;

	/* Disable USB PHY and PLL */
	lpc_scb.pdruncfg |= ((1 << LPC_SCB_PDRUNCFG_USBPAD_PD) |
			     (1 << LPC_SCB_PDRUNCFG_USBPLL_PD));

	/* Disable USB registers and RAM */
	lpc_scb.sysahbclkctrl &= ~((1 << LPC_SCB_SYSAHBCLKCTRL_USB) |
				   (1 << LPC_SCB_SYSAHBCLKCTRL_USBRAM));

	ao_arch_release_interrupts();
}

void
ao_usb_enable(void)
{
	int	t;

	/* Enable USB pins */
#if HAS_LPC_USB_CONNECT
	lpc_ioconf.pio0_6 = ((LPC_IOCONF_FUNC_USB_CONNECT << LPC_IOCONF_FUNC) |
			     (LPC_IOCONF_MODE_INACTIVE << LPC_IOCONF_MODE) |
			     (0 << LPC_IOCONF_HYS) |
			     (0 << LPC_IOCONF_INV) |
			     (0 << LPC_IOCONF_OD) |
			     0x80);
#endif
#if HAS_USB_VBUS
	lpc_ioconf.pio0_3 = ((LPC_IOCONF_FUNC_USB_VBUS << LPC_IOCONF_FUNC) |
			     (LPC_IOCONF_MODE_INACTIVE << LPC_IOCONF_MODE) |
			     (0 << LPC_IOCONF_HYS) |
			     (0 << LPC_IOCONF_INV) |
			     (0 << LPC_IOCONF_OD) |
			     0x80);
#endif
	/* Enable USB registers and RAM */
	lpc_scb.sysahbclkctrl |= ((1 << LPC_SCB_SYSAHBCLKCTRL_USB) |
				  (1 << LPC_SCB_SYSAHBCLKCTRL_USBRAM));

	/* Enable USB PHY */
	lpc_scb.pdruncfg &= ~(1 << LPC_SCB_PDRUNCFG_USBPAD_PD);
	
	/* Turn on USB PLL */
	lpc_scb.pdruncfg &= ~(1 << LPC_SCB_PDRUNCFG_USBPLL_PD);

	lpc_scb.usbpllclksel = (LPC_SCB_SYSPLLCLKSEL_SEL_SYSOSC << LPC_SCB_SYSPLLCLKSEL_SEL);
	lpc_scb.usbpllclkuen = (0 << LPC_SCB_USBPLLCLKUEN_ENA);
	lpc_scb.usbpllclkuen = (1 << LPC_SCB_USBPLLCLKUEN_ENA);
	while (!(lpc_scb.usbpllclkuen & (1 << LPC_SCB_USBPLLCLKUEN_ENA)))
		;
	lpc_scb.usbpllctrl = 0x23;
	while (!(lpc_scb.usbpllstat & 1))
		;

	lpc_scb.usbclksel = 0;
	lpc_scb.usbclkuen = (0 << LPC_SCB_USBCLKUEN_ENA);
	lpc_scb.usbclkuen = (1 << LPC_SCB_USBCLKUEN_ENA);
	while (!(lpc_scb.usbclkuen & (1 << LPC_SCB_USBCLKUEN_ENA)))
		;

	/* Turn on USB clock, use 48MHz clock unchanged */
	lpc_scb.usbclkdiv = 1;

	/* Configure interrupts */
	ao_arch_block_interrupts();

	/* Route all interrupts to the main isr */
	lpc_usb.introuting = 0;

	/* Configure NVIC */

	lpc_nvic_set_enable(LPC_ISR_USB_IRQ_POS);
	lpc_nvic_set_priority(LPC_ISR_USB_IRQ_POS, 0);

	/* Clear any spurious interrupts */
	lpc_usb.intstat = 0xffffffff;

	debug ("ao_usb_enable\n");

	/* Enable interrupts */
	lpc_usb.inten = ((1 << LPC_USB_INT_EPOUT(0)) |
			 (1 << LPC_USB_INT_EPIN(0)) |
			 (1 << LPC_USB_INT_EPIN(AO_USB_INT_EP)) |
			 (1 << LPC_USB_INT_EPOUT(AO_USB_OUT_EP)) |
			 (1 << LPC_USB_INT_EPIN(AO_USB_IN_EP)) |
			 (1 << LPC_USB_INT_DEV));

	ao_arch_release_interrupts();

	lpc_usb.devcmdstat = 0;
	for (t = 0; t < 1000; t++)
		ao_arch_nop();

	ao_usb_set_ep0();

#if HAS_USB_PULLUP
	ao_gpio_set(AO_USB_PULLUP_PORT, AO_USB_PULLUP_PIN, AO_USB_PULLUP, 1);
#endif
}

#if USB_ECHO
struct ao_task ao_usb_echo_task;

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

#if USB_DEBUG
static void
ao_usb_irq(void)
{
	printf ("control: %d out: %d in: %d int: %d reset: %d\n",
		control_count, out_count, in_count, int_count, reset_count);
}

__code struct ao_cmds ao_usb_cmds[] = {
	{ ao_usb_irq, "I\0Show USB interrupt counts" },
	{ 0, NULL }
};
#endif

void
ao_usb_init(void)
{
#if HAS_USB_PULLUP
	ao_enable_output(AO_USB_PULLUP_PORT, AO_USB_PULLUP_PIN, AO_USB_PULLUP, 0);
#endif

	ao_usb_enable();

	debug ("ao_usb_init\n");
#if USB_ECHO
	ao_add_task(&ao_usb_echo_task, ao_usb_echo, "usb echo");
#endif
#if USB_DEBUG
	ao_cmd_register(&ao_usb_cmds[0]);
#endif
#if USE_USB_STDIO
	ao_add_stdio(_ao_usb_pollchar, ao_usb_putchar, ao_usb_flush);
#endif
}

#if TX_DBG || RX_DBG

struct ao_usb_dbg {
	int		line;
	char		*msg;
	uint32_t	value;
	uint32_t	primask;
#if TX_DBG
	uint16_t	in_count;
	uint32_t	in_ep;
	uint32_t	in_pending;
	uint32_t	tx_count;
	uint32_t	in_flushed;
#endif
#if RX_DBG
	uint8_t		rx_count;
	uint8_t		rx_pos;
	uint8_t		out_avail;
	uint32_t	out_ep;
#endif
};

#define NUM_USB_DBG	8

static struct ao_usb_dbg dbg[NUM_USB_DBG];
static int dbg_i;

static void _dbg(int line, char *msg, uint32_t value)
{
	uint32_t	primask;
	dbg[dbg_i].line = line;
	dbg[dbg_i].msg = msg;
	dbg[dbg_i].value = value;
	asm("mrs %0,primask" : "=&r" (primask));
	dbg[dbg_i].primask = primask;
#if TX_DBG
	dbg[dbg_i].in_count = in_count;
	dbg[dbg_i].in_ep = *ao_usb_epn_in(AO_USB_IN_EP);
	dbg[dbg_i].in_pending = ao_usb_in_pending;
	dbg[dbg_i].tx_count = ao_usb_tx_count;
	dbg[dbg_i].in_flushed = ao_usb_in_flushed;
#endif
#if RX_DBG
	dbg[dbg_i].rx_count = ao_usb_rx_count;
	dbg[dbg_i].rx_pos = ao_usb_rx_pos;
	dbg[dbg_i].out_avail = ao_usb_out_avail;
	dbg[dbg_i].out_ep = *ao_usb_epn_out(AO_USB_OUT_EP);
#endif
	if (++dbg_i == NUM_USB_DBG)
		dbg_i = 0;
}
#endif

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

#define USB_DEBUG 	0
#define USB_DEBUG_DATA	0
#define USB_ECHO	0

#ifndef USE_USB_STDIO
#define USE_USB_STDIO	1
#endif

#if USE_USB_STDIO
#define AO_USB_OUT_SLEEP_ADDR	(&ao_stdin_ready)
#else
#define AO_USB_OUT_SLEEP_ADDR	(&ao_usb_out_avail)
#endif

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

/* Temp buffer for smaller EP0 in data */
static uint8_t	ao_usb_ep0_in_buf[2];

/* Pending EP0 OUT data */
static uint8_t *ao_usb_ep0_out_data;
static uint8_t 	ao_usb_ep0_out_len;

/*
 * Objects allocated in special USB memory
 */

/* Buffer description tables */
static union stm_usb_bdt	*ao_usb_bdt;
/* USB address of end of allocated storage */
static uint16_t	ao_usb_sram_addr;

/* Pointer to ep0 tx/rx buffers in USB memory */
static uint32_t	*ao_usb_ep0_tx_buffer;
static uint32_t	*ao_usb_ep0_rx_buffer;

/* Pointer to bulk data tx/rx buffers in USB memory */
static uint32_t	*ao_usb_in_tx_buffer;
static uint32_t	*ao_usb_out_rx_buffer;

/* System ram shadow of USB buffer; writing individual bytes is
 * too much of a pain (sigh) */
static uint8_t	ao_usb_tx_buffer[AO_USB_IN_SIZE];
static uint8_t	ao_usb_tx_count;

static uint8_t	ao_usb_rx_buffer[AO_USB_OUT_SIZE];
static uint8_t	ao_usb_rx_count, ao_usb_rx_pos;

/*
 * End point register indices
 */

#define AO_USB_CONTROL_EPR	0
#define AO_USB_INT_EPR		1
#define AO_USB_OUT_EPR		2
#define AO_USB_IN_EPR		3

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

static inline uint32_t *ao_usb_packet_buffer_addr(uint16_t sram_addr)
{
	return (uint32_t *) (stm_usb_sram + 2 * sram_addr);
}

static inline uint32_t ao_usb_epr_stat_rx(uint32_t epr) {
	return (epr >> STM_USB_EPR_STAT_RX) & STM_USB_EPR_STAT_RX_MASK;
}

static inline uint32_t ao_usb_epr_stat_tx(uint32_t epr) {
	return (epr >> STM_USB_EPR_STAT_TX) & STM_USB_EPR_STAT_TX_MASK;
}

static inline uint32_t ao_usb_epr_ctr_rx(uint32_t epr) {
	return (epr >> STM_USB_EPR_CTR_RX) & 1;
}

static inline uint32_t ao_usb_epr_ctr_tx(uint32_t epr) {
	return (epr >> STM_USB_EPR_CTR_TX) & 1;
}

static inline uint32_t ao_usb_epr_setup(uint32_t epr) {
	return (epr >> STM_USB_EPR_SETUP) & 1;
}

static inline uint32_t ao_usb_epr_dtog_rx(uint32_t epr) {
	return (epr >> STM_USB_EPR_DTOG_RX) & 1;
}

static inline uint32_t ao_usb_epr_dtog_tx(uint32_t epr) {
	return (epr >> STM_USB_EPR_DTOG_TX) & 1;
}

/*
 * Set current device address and mark the
 * interface as active
 */
void
ao_usb_set_address(uint8_t address)
{
	debug("ao_usb_set_address %02x\n", address);
	stm_usb.daddr = (1 << STM_USB_DADDR_EF) | address;
	ao_usb_address_pending = 0;
}

/*
 * Write these values to preserve register contents under HW changes
 */

#define STM_USB_EPR_INVARIANT	((1 << STM_USB_EPR_CTR_RX) |		\
				 (STM_USB_EPR_DTOG_RX_WRITE_INVARIANT << STM_USB_EPR_DTOG_RX) | \
				 (STM_USB_EPR_STAT_RX_WRITE_INVARIANT << STM_USB_EPR_STAT_RX) | \
				 (1 << STM_USB_EPR_CTR_TX) |		\
				 (STM_USB_EPR_DTOG_TX_WRITE_INVARIANT << STM_USB_EPR_DTOG_TX) |	\
				 (STM_USB_EPR_STAT_TX_WRITE_INVARIANT << STM_USB_EPR_STAT_TX))

#define STM_USB_EPR_INVARIANT_MASK	((1 << STM_USB_EPR_CTR_RX) |	\
					 (STM_USB_EPR_DTOG_RX_MASK << STM_USB_EPR_DTOG_RX) | \
					 (STM_USB_EPR_STAT_RX_MASK << STM_USB_EPR_STAT_RX) | \
					 (1 << STM_USB_EPR_CTR_TX) |	\
					 (STM_USB_EPR_DTOG_TX_MASK << STM_USB_EPR_DTOG_TX) | \
					 (STM_USB_EPR_STAT_TX_MASK << STM_USB_EPR_STAT_TX))

/*
 * These bits are purely under sw control, so preserve them in the
 * register by re-writing what was read
 */
#define STM_USB_EPR_PRESERVE_MASK	((STM_USB_EPR_EP_TYPE_MASK << STM_USB_EPR_EP_TYPE) | \
					 (1 << STM_USB_EPR_EP_KIND) |	\
					 (STM_USB_EPR_EA_MASK << STM_USB_EPR_EA))

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
 * Set the state of the specified endpoint register to a new
 * value. This is tricky because the bits toggle where the new
 * value is one, and we need to write invariant values in other
 * spots of the register. This hardware is strange...
 */
static void
_ao_usb_set_stat_tx(int ep, uint32_t stat_tx)
{
	uint32_t	epr_write, epr_old;

	_tx_dbg1("set_stat_tx top", stat_tx);
	epr_old = epr_write = stm_usb.epr[ep];
	epr_write &= STM_USB_EPR_PRESERVE_MASK;
	epr_write |= STM_USB_EPR_INVARIANT;
	epr_write |= set_toggle(epr_old,
			      STM_USB_EPR_STAT_TX_MASK << STM_USB_EPR_STAT_TX,
			      stat_tx << STM_USB_EPR_STAT_TX);
	stm_usb.epr[ep] = epr_write;
	_tx_dbg1("set_stat_tx bottom", epr_write);
}

static void
ao_usb_set_stat_tx(int ep, uint32_t stat_tx)
{
	ao_arch_block_interrupts();
	_ao_usb_set_stat_tx(ep, stat_tx);
	ao_arch_release_interrupts();
}

static void
_ao_usb_set_stat_rx(int ep, uint32_t stat_rx) {
	uint32_t	epr_write, epr_old;

	epr_write = epr_old = stm_usb.epr[ep];
	epr_write &= STM_USB_EPR_PRESERVE_MASK;
	epr_write |= STM_USB_EPR_INVARIANT;
	epr_write |= set_toggle(epr_old,
			      STM_USB_EPR_STAT_RX_MASK << STM_USB_EPR_STAT_RX,
			      stat_rx << STM_USB_EPR_STAT_RX);
	stm_usb.epr[ep] = epr_write;
}

static void
ao_usb_set_stat_rx(int ep, uint32_t stat_rx) {
	ao_arch_block_interrupts();
	_ao_usb_set_stat_rx(ep, stat_rx);
	ao_arch_release_interrupts();
}

/*
 * Set just endpoint 0, for use during startup
 */

static void
ao_usb_init_ep(uint8_t ep, uint32_t addr, uint32_t type, uint32_t stat_rx, uint32_t stat_tx)
{
	uint32_t		epr;
	ao_arch_block_interrupts();
	epr = stm_usb.epr[ep];
	epr = ((0 << STM_USB_EPR_CTR_RX) |
	       (epr & (1 << STM_USB_EPR_DTOG_RX)) |
	       set_toggle(epr,
			  (STM_USB_EPR_STAT_RX_MASK << STM_USB_EPR_STAT_RX),
			  (stat_rx << STM_USB_EPR_STAT_RX)) |
	       (type << STM_USB_EPR_EP_TYPE) |
	       (0 << STM_USB_EPR_EP_KIND) |
	       (0 << STM_USB_EPR_CTR_TX) |
	       (epr & (1 << STM_USB_EPR_DTOG_TX)) |
	       set_toggle(epr,
			  (STM_USB_EPR_STAT_TX_MASK << STM_USB_EPR_STAT_TX),
			  (stat_tx << STM_USB_EPR_STAT_TX)) |
	       (addr << STM_USB_EPR_EA));
	stm_usb.epr[ep] = epr;
	ao_arch_release_interrupts();
	debug ("writing epr[%d] 0x%08x wrote 0x%08x\n",
	       ep, epr, stm_usb.epr[ep]);
}

static void
ao_usb_set_ep0(void)
{
	int			e;

	ao_usb_sram_addr = 0;

	/* buffer table is at the start of USB memory */
	stm_usb.btable = 0;
	ao_usb_bdt = (void *) stm_usb_sram;

	ao_usb_sram_addr += 8 * STM_USB_BDT_SIZE;

	/* Set up EP 0 - a Control end point with 32 bytes of in and out buffers */

	ao_usb_bdt[0].single.addr_tx = ao_usb_sram_addr;
	ao_usb_bdt[0].single.count_tx = 0;
	ao_usb_ep0_tx_buffer = ao_usb_packet_buffer_addr(ao_usb_sram_addr);
	ao_usb_sram_addr += AO_USB_CONTROL_SIZE;

	ao_usb_bdt[0].single.addr_rx = ao_usb_sram_addr;
	ao_usb_bdt[0].single.count_rx = ((1 << STM_USB_BDT_COUNT_RX_BL_SIZE) |
				  (((AO_USB_CONTROL_SIZE / 32) - 1) << STM_USB_BDT_COUNT_RX_NUM_BLOCK));
	ao_usb_ep0_rx_buffer = ao_usb_packet_buffer_addr(ao_usb_sram_addr);
	ao_usb_sram_addr += AO_USB_CONTROL_SIZE;

	ao_usb_init_ep(AO_USB_CONTROL_EPR, AO_USB_CONTROL_EP,
		       STM_USB_EPR_EP_TYPE_CONTROL,
		       STM_USB_EPR_STAT_RX_VALID,
		       STM_USB_EPR_STAT_TX_NAK);

	/* Clear all of the other endpoints */
	for (e = 1; e < 8; e++) {
		ao_usb_init_ep(e, 0,
			       STM_USB_EPR_EP_TYPE_CONTROL,
			       STM_USB_EPR_STAT_RX_DISABLED,
			       STM_USB_EPR_STAT_TX_DISABLED);
	}

	ao_usb_set_address(0);
}

static void
ao_usb_set_configuration(void)
{
	debug ("ao_usb_set_configuration\n");

	/* Set up the INT end point */
	ao_usb_bdt[AO_USB_INT_EPR].single.addr_tx = ao_usb_sram_addr;
	ao_usb_bdt[AO_USB_INT_EPR].single.count_tx = 0;
	ao_usb_in_tx_buffer = ao_usb_packet_buffer_addr(ao_usb_sram_addr);
	ao_usb_sram_addr += AO_USB_INT_SIZE;

	ao_usb_init_ep(AO_USB_INT_EPR,
		       AO_USB_INT_EP,
		       STM_USB_EPR_EP_TYPE_INTERRUPT,
		       STM_USB_EPR_STAT_RX_DISABLED,
		       STM_USB_EPR_STAT_TX_NAK);

	/* Set up the OUT end point */
	ao_usb_bdt[AO_USB_OUT_EPR].single.addr_rx = ao_usb_sram_addr;
	ao_usb_bdt[AO_USB_OUT_EPR].single.count_rx = ((1 << STM_USB_BDT_COUNT_RX_BL_SIZE) |
						      (((AO_USB_OUT_SIZE / 32) - 1) << STM_USB_BDT_COUNT_RX_NUM_BLOCK));
	ao_usb_out_rx_buffer = ao_usb_packet_buffer_addr(ao_usb_sram_addr);
	ao_usb_sram_addr += AO_USB_OUT_SIZE;

	ao_usb_init_ep(AO_USB_OUT_EPR,
		       AO_USB_OUT_EP,
		       STM_USB_EPR_EP_TYPE_BULK,
		       STM_USB_EPR_STAT_RX_VALID,
		       STM_USB_EPR_STAT_TX_DISABLED);

	/* Set up the IN end point */
	ao_usb_bdt[AO_USB_IN_EPR].single.addr_tx = ao_usb_sram_addr;
	ao_usb_bdt[AO_USB_IN_EPR].single.count_tx = 0;
	ao_usb_in_tx_buffer = ao_usb_packet_buffer_addr(ao_usb_sram_addr);
	ao_usb_sram_addr += AO_USB_IN_SIZE;

	ao_usb_init_ep(AO_USB_IN_EPR,
		       AO_USB_IN_EP,
		       STM_USB_EPR_EP_TYPE_BULK,
		       STM_USB_EPR_STAT_RX_DISABLED,
		       STM_USB_EPR_STAT_TX_NAK);

	ao_usb_running = 1;
}

static uint16_t	control_count;
static uint16_t int_count;
static uint16_t	in_count;
static uint16_t	out_count;
static uint16_t	reset_count;

/* The USB memory holds 16 bit values on 32 bit boundaries
 * and must be accessed only in 32 bit units. Sigh.
 */

static inline void
ao_usb_write_byte(uint8_t byte, uint32_t *base, uint16_t offset)
{
	base += offset >> 1;
	if (offset & 1) {
		*base = (*base & 0xff) | ((uint32_t) byte << 8);
	} else {
		*base = (*base & 0xff00) | byte;
	}
}

static inline void
ao_usb_write_short(uint16_t data, uint32_t *base, uint16_t offset)
{
	base[offset>>1] = data;
}

static void
ao_usb_write(const uint8_t *src, uint32_t *base, uint16_t offset, uint16_t bytes)
{
	if (!bytes)
		return;
	if (offset & 1) {
		debug_data (" %02x", src[0]);
		ao_usb_write_byte(*src++, base, offset++);
		bytes--;
	}
	while (bytes >= 2) {
		debug_data (" %02x %02x", src[0], src[1]);
		ao_usb_write_short((src[1] << 8) | src[0], base, offset);
		offset += 2;
		src += 2;
		bytes -= 2;
	}
	if (bytes) {
		debug_data (" %02x", src[0]);
		ao_usb_write_byte(*src, base, offset);
	}
}

static inline uint8_t
ao_usb_read_byte(uint32_t *base, uint16_t offset)
{
	base += offset >> 1;
	if (offset & 1)
		return (*base >> 8) & 0xff;
	else
		return *base & 0xff;
}

static inline uint16_t
ao_usb_read_short(uint32_t *base, uint16_t offset)
{
	return base[offset>>1];
}

static void
ao_usb_read(uint8_t *dst, uint32_t *base, uint16_t offset, uint16_t bytes)
{
	if (!bytes)
		return;
	if (offset & 1) {
		*dst++ = ao_usb_read_byte(base, offset++);
		debug_data (" %02x", dst[-1]);
		bytes--;
	}
	while (bytes >= 2) {
		uint16_t	s = ao_usb_read_short(base, offset);
		dst[0] = s;
		dst[1] = s >> 8;
		debug_data (" %02x %02x", dst[0], dst[1]);
		offset += 2;
		dst += 2;
		bytes -= 2;
	}
	if (bytes) {
		*dst = ao_usb_read_byte(base, offset);
		debug_data (" %02x", dst[0]);
	}
}

/* Send an IN data packet */
static void
ao_usb_ep0_flush(void)
{
	uint8_t this_len;

	/* Check to see if the endpoint is still busy */
	if (ao_usb_epr_stat_tx(stm_usb.epr[0]) == STM_USB_EPR_STAT_TX_VALID) {
		debug("EP0 not accepting IN data\n");
		return;
	}

	this_len = ao_usb_ep0_in_len;
	if (this_len > AO_USB_CONTROL_SIZE)
		this_len = AO_USB_CONTROL_SIZE;

	if (this_len < AO_USB_CONTROL_SIZE)
		ao_usb_ep0_state = AO_USB_EP0_IDLE;

	ao_usb_ep0_in_len -= this_len;

	debug_data ("Flush EP0 len %d:", this_len);
	ao_usb_write(ao_usb_ep0_in_data, ao_usb_ep0_tx_buffer, 0, this_len);
	debug_data ("\n");
	ao_usb_ep0_in_data += this_len;

	/* Mark the endpoint as TX valid to send the packet */
	ao_usb_bdt[AO_USB_CONTROL_EPR].single.count_tx = this_len;
	ao_usb_set_stat_tx(AO_USB_CONTROL_EPR, STM_USB_EPR_STAT_TX_VALID);
	debug ("queue tx. epr 0 now %08x\n", stm_usb.epr[AO_USB_CONTROL_EPR]);
}

/* Read data from the ep0 OUT fifo */
static void
ao_usb_ep0_fill(void)
{
	uint16_t	len = ao_usb_bdt[0].single.count_rx & STM_USB_BDT_COUNT_RX_COUNT_RX_MASK;

	if (len > ao_usb_ep0_out_len)
		len = ao_usb_ep0_out_len;
	ao_usb_ep0_out_len -= len;

	/* Pull all of the data out of the packet */
	debug_data ("Fill EP0 len %d:", len);
	ao_usb_read(ao_usb_ep0_out_data, ao_usb_ep0_rx_buffer, 0, len);
	debug_data ("\n");
	ao_usb_ep0_out_data += len;

	/* ACK the packet */
	ao_usb_set_stat_rx(0, STM_USB_EPR_STAT_RX_VALID);
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
		ao_usb_set_ep0();
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

#if HAS_FLIGHT && AO_USB_FORCE_IDLE
		ao_flight_force_idle = 1;
#endif
		/* Wait until the IN packet is received from addr 0
		 * before assigning our local address
		 */
		if (ao_usb_address_pending)
			ao_usb_set_address(ao_usb_address);
		if (ao_usb_ep0_state == AO_USB_EP0_DATA_IN)
			ao_usb_ep0_flush();
	}
}

void
stm_usb_lp_isr(void)
{
	uint32_t	istr = stm_usb.istr;

	if (istr & (1 << STM_USB_ISTR_CTR)) {
		uint8_t		ep = istr & STM_USB_ISTR_EP_ID_MASK;
		uint32_t	epr, epr_write;

		/* Preserve the SW write bits, don't mess with most HW writable bits,
		 * clear the CTR_RX and CTR_TX bits
		 */
		epr = stm_usb.epr[ep];
		epr_write = epr;
		epr_write &= STM_USB_EPR_PRESERVE_MASK;
		epr_write |= STM_USB_EPR_INVARIANT;
		epr_write &= ~(1 << STM_USB_EPR_CTR_RX);
		epr_write &= ~(1 << STM_USB_EPR_CTR_TX);
		stm_usb.epr[ep] = epr_write;

		switch (ep) {
		case 0:
			++control_count;
			if (ao_usb_epr_ctr_rx(epr)) {
				if (ao_usb_epr_setup(epr))
					ao_usb_ep0_receive |= AO_USB_EP0_GOT_SETUP;
				else
					ao_usb_ep0_receive |= AO_USB_EP0_GOT_RX_DATA;
			}
			if (ao_usb_epr_ctr_tx(epr))
				ao_usb_ep0_receive |= AO_USB_EP0_GOT_TX_ACK;
			ao_usb_ep0_handle(ao_usb_ep0_receive);
			break;
		case AO_USB_OUT_EPR:
			++out_count;
			if (ao_usb_epr_ctr_rx(epr)) {
				_rx_dbg1("RX ISR", epr);
				ao_usb_out_avail = 1;
				_rx_dbg0("out avail set");
				ao_wakeup(AO_USB_OUT_SLEEP_ADDR);
				_rx_dbg0("stdin awoken");
			}
			break;
		case AO_USB_IN_EPR:
			++in_count;
			_tx_dbg1("TX ISR", epr);
			if (ao_usb_epr_ctr_tx(epr)) {
				ao_usb_in_pending = 0;
				ao_wakeup(&ao_usb_in_pending);
			}
			break;
		case AO_USB_INT_EPR:
			++int_count;
			if (ao_usb_epr_ctr_tx(epr))
				_ao_usb_set_stat_tx(AO_USB_INT_EPR, STM_USB_EPR_STAT_TX_NAK);
			break;
		}
		return;
	}

	if (istr & (1 << STM_USB_ISTR_RESET)) {
		++reset_count;
		stm_usb.istr &= ~(1 << STM_USB_ISTR_RESET);
		ao_usb_ep0_receive |= AO_USB_EP0_GOT_RESET;
		ao_usb_ep0_handle(ao_usb_ep0_receive);
	}
}

void
stm_usb_fs_wkup(void)
{
	/* USB wakeup, just clear the bit for now */
	stm_usb.istr &= ~(1 << STM_USB_ISTR_WKUP);
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
	ao_usb_write(ao_usb_tx_buffer, ao_usb_in_tx_buffer, 0, ao_usb_tx_count);
	ao_usb_bdt[AO_USB_IN_EPR].single.count_tx = ao_usb_tx_count;
	ao_usb_tx_count = 0;
	_ao_usb_set_stat_tx(AO_USB_IN_EPR, STM_USB_EPR_STAT_TX_VALID);
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

	ao_usb_rx_count = ao_usb_bdt[AO_USB_OUT_EPR].single.count_rx & STM_USB_BDT_COUNT_RX_COUNT_RX_MASK;

	_rx_dbg1("out_recv count", ao_usb_rx_count);
	debug ("recv %d\n", ao_usb_rx_count);
	debug_data("Fill OUT len %d:", ao_usb_rx_count);
	ao_usb_read(ao_usb_rx_buffer, ao_usb_out_rx_buffer, 0, ao_usb_rx_count);
	debug_data("\n");
	ao_usb_rx_pos = 0;

	/* ACK the packet */
	_ao_usb_set_stat_rx(AO_USB_OUT_EPR, STM_USB_EPR_STAT_RX_VALID);
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
	stm_usb.cntr = (1 << STM_USB_CNTR_FRES);
	stm_usb.istr = 0;

	/* Disable USB pull-up */
	stm_syscfg.pmc &= ~(1 << STM_SYSCFG_PMC_USB_PU);

	/* Switch off the device */
	stm_usb.cntr = (1 << STM_USB_CNTR_PDWN) | (1 << STM_USB_CNTR_FRES);

	/* Disable the interface */
	stm_rcc.apb1enr &= ~(1 << STM_RCC_APB1ENR_USBEN);
	ao_arch_release_interrupts();
}

void
ao_usb_enable(void)
{
	int	t;

	/* Enable SYSCFG */
	stm_rcc.apb2enr |= (1 << STM_RCC_APB2ENR_SYSCFGEN);

	/* Disable USB pull-up */
	stm_syscfg.pmc &= ~(1 << STM_SYSCFG_PMC_USB_PU);

	/* Enable USB device */
	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_USBEN);

	/* Do not touch the GPIOA configuration; USB takes priority
	 * over GPIO on pins A11 and A12, but if you select alternate
	 * input 10 (the documented correct selection), then USB is
	 * pulled low and doesn't work at all
	 */

	ao_arch_block_interrupts();

	/* Route interrupts */
	stm_nvic_set_priority(STM_ISR_USB_LP_POS, 3);
	stm_nvic_set_enable(STM_ISR_USB_LP_POS);

	ao_usb_configuration = 0;

	stm_usb.cntr = (1 << STM_USB_CNTR_FRES);

	/* Clear the power down bit */
	stm_usb.cntr = 0;

	/* Clear any spurious interrupts */
	stm_usb.istr = 0;

	debug ("ao_usb_enable\n");

	/* Enable interrupts */
	stm_usb.cntr = ((1 << STM_USB_CNTR_CTRM) |
			(0 << STM_USB_CNTR_PMAOVRM) |
			(0 << STM_USB_CNTR_ERRM) |
			(0 << STM_USB_CNTR_WKUPM) |
			(0 << STM_USB_CNTR_SUSPM) |
			(1 << STM_USB_CNTR_RESETM) |
			(0 << STM_USB_CNTR_SOFM) |
			(0 << STM_USB_CNTR_ESOFM) |
			(0 << STM_USB_CNTR_RESUME) |
			(0 << STM_USB_CNTR_FSUSP) |
			(0 << STM_USB_CNTR_LP_MODE) |
			(0 << STM_USB_CNTR_PDWN) |
			(0 << STM_USB_CNTR_FRES));

	ao_arch_release_interrupts();

	for (t = 0; t < 1000; t++)
		ao_arch_nop();
	/* Enable USB pull-up */
	stm_syscfg.pmc |= (1 << STM_SYSCFG_PMC_USB_PU);
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
	ao_usb_enable();

	debug ("ao_usb_init\n");
	ao_usb_ep0_state = AO_USB_EP0_IDLE;
#if USB_ECHO
	ao_add_task(&ao_usb_echo_task, ao_usb_echo, "usb echo");
#endif
#if USB_DEBUG
	ao_cmd_register(&ao_usb_cmds[0]);
#endif
#if !USB_ECHO
#if USE_USB_STDIO
	ao_add_stdio(_ao_usb_pollchar, ao_usb_putchar, ao_usb_flush);
#endif
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
	uint32_t	in_epr;
	uint32_t	in_pending;
	uint32_t	tx_count;
	uint32_t	in_flushed;
#endif
#if RX_DBG
	uint8_t		rx_count;
	uint8_t		rx_pos;
	uint8_t		out_avail;
	uint32_t	out_epr;
#endif
};

#define NUM_USB_DBG	128

static struct ao_usb_dbg dbg[128];
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
	dbg[dbg_i].in_epr = stm_usb.epr[AO_USB_IN_EPR];
	dbg[dbg_i].in_pending = ao_usb_in_pending;
	dbg[dbg_i].tx_count = ao_usb_tx_count;
	dbg[dbg_i].in_flushed = ao_usb_in_flushed;
#endif
#if RX_DBG
	dbg[dbg_i].rx_count = ao_usb_rx_count;
	dbg[dbg_i].rx_pos = ao_usb_rx_pos;
	dbg[dbg_i].out_avail = ao_usb_out_avail;
	dbg[dbg_i].out_epr = stm_usb.epr[AO_USB_OUT_EPR];
#endif
	if (++dbg_i == NUM_USB_DBG)
		dbg_i = 0;
}
#endif

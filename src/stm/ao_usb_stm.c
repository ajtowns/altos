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

struct ao_task ao_usb_task;

struct ao_usb_setup {
	uint8_t		dir_type_recip;
	uint8_t		request;
	uint16_t	value;
	uint16_t	index;
	uint16_t	length;
} ao_usb_setup;

static uint8_t 	ao_usb_ep0_state;
static const uint8_t * ao_usb_ep0_in_data;
static uint8_t 	ao_usb_ep0_in_len;
static uint8_t	ao_usb_ep0_in_pending;
static uint8_t	ao_usb_ep0_in_buf[2];
static uint8_t 	ao_usb_ep0_out_len;
static uint8_t *ao_usb_ep0_out_data;
static union stm_usb_bdt	*ao_usb_bdt;
static uint16_t	ao_usb_sram_addr;
static uint8_t	ao_usb_tx_buffer[AO_USB_IN_SIZE];
static uint8_t	ao_usb_tx_count;
static uint8_t	ao_usb_rx_buffer[AO_USB_OUT_SIZE];
static uint8_t	ao_usb_rx_count, ao_usb_rx_pos;

#define AO_USB_INT_EPR	1
#define AO_USB_OUT_EPR	2
#define AO_USB_IN_EPR	3

/*
 * Pointers into the USB packet buffer area
 */
static uint32_t	*ao_usb_ep0_tx_buffer;
static uint32_t	*ao_usb_ep0_rx_buffer;

static uint32_t	*ao_usb_in_tx_buffer;
static uint32_t	*ao_usb_out_rx_buffer;

static uint8_t	ao_usb_in_flushed;
static uint8_t	ao_usb_in_pending;
static uint8_t	ao_usb_out_avail;
static uint8_t	ao_usb_running;
static uint8_t	ao_usb_configuration;
static uint8_t	ueienx_0;

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
 * Set just endpoint 0, for use during startup
 */

static void
ao_usb_set_ep0(void)
{
	uint32_t		epr;
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

	cli();
	epr = stm_usb.epr[0];
	epr = ((STM_USB_EPR_CTR_RX_WRITE_INVARIANT << STM_USB_EPR_CTR_RX) |
	       (STM_USB_EPR_DTOG_RX_WRITE_INVARIANT << STM_USB_EPR_DTOG_RX) |
	       set_toggle(epr,
			  (STM_USB_EPR_STAT_RX_MASK << STM_USB_EPR_STAT_RX),
			  (STM_USB_EPR_STAT_RX_VALID << STM_USB_EPR_STAT_RX)) |
	       (STM_USB_EPR_EP_TYPE_CONTROL << STM_USB_EPR_EP_TYPE) |
	       (0 << STM_USB_EPR_EP_KIND) |
	       (STM_USB_CTR_TX_WRITE_INVARIANT << STM_USB_EPR_CTR_TX) |
	       (STM_USB_EPR_DTOG_TX_WRITE_INVARIANT << STM_USB_EPR_DTOG_TX) |
	       set_toggle(epr,
			  (STM_USB_EPR_STAT_TX_MASK << STM_USB_EPR_STAT_TX),
			  (STM_USB_EPR_STAT_TX_NAK << STM_USB_EPR_STAT_TX)) |
	       (AO_USB_CONTROL_EP << STM_USB_EPR_EA));
	stm_usb.epr[0] = epr;
	sei();
	debug ("epr 0 now %x\n", stm_usb.epr[0]);

	/* Clear all of the other endpoints */
	for (e = 1; e < 8; e++) {
		cli();
		epr = stm_usb.epr[e];
		epr = ((STM_USB_EPR_CTR_RX_WRITE_INVARIANT << STM_USB_EPR_CTR_RX) |
		       (STM_USB_EPR_DTOG_RX_WRITE_INVARIANT << STM_USB_EPR_DTOG_RX) |
		       set_toggle(epr,
				  (STM_USB_EPR_STAT_RX_MASK << STM_USB_EPR_STAT_RX),
				  (STM_USB_EPR_STAT_RX_DISABLED << STM_USB_EPR_STAT_RX)) |
		       (STM_USB_EPR_EP_TYPE_CONTROL << STM_USB_EPR_EP_TYPE) |
		       (0 << STM_USB_EPR_EP_KIND) |
		       (STM_USB_CTR_TX_WRITE_INVARIANT << STM_USB_EPR_CTR_TX) |
		       (STM_USB_EPR_DTOG_TX_WRITE_INVARIANT << STM_USB_EPR_DTOG_TX) |
		       set_toggle(epr,
				  (STM_USB_EPR_STAT_TX_MASK << STM_USB_EPR_STAT_TX),
				  (STM_USB_EPR_STAT_TX_DISABLED << STM_USB_EPR_STAT_TX)) |
		       (0 << STM_USB_EPR_EA));
		stm_usb.epr[e] = epr;
		sei();
	}

	ao_usb_set_address(0);
}

static void
ao_usb_set_configuration(void)
{
	uint32_t		epr;

	debug ("ao_usb_set_configuration\n");

	/* Set up the INT end point */
	ao_usb_bdt[AO_USB_INT_EPR].single.addr_tx = ao_usb_sram_addr;
	ao_usb_bdt[AO_USB_INT_EPR].single.count_tx = 0;
	ao_usb_in_tx_buffer = ao_usb_packet_buffer_addr(ao_usb_sram_addr);
	ao_usb_sram_addr += AO_USB_INT_SIZE;

	cli();
	epr = stm_usb.epr[AO_USB_INT_EPR];
	epr = ((0 << STM_USB_EPR_CTR_RX) |
	       (epr & (1 << STM_USB_EPR_DTOG_RX)) |
	       set_toggle(epr,
			  (STM_USB_EPR_STAT_RX_MASK << STM_USB_EPR_STAT_RX),
			  (STM_USB_EPR_STAT_RX_DISABLED << STM_USB_EPR_STAT_RX)) |
	       (STM_USB_EPR_EP_TYPE_CONTROL << STM_USB_EPR_EP_TYPE) |
	       (0 << STM_USB_EPR_EP_KIND) |
	       (0 << STM_USB_EPR_CTR_TX) |
	       (epr & (1 << STM_USB_EPR_DTOG_TX)) |
	       set_toggle(epr,
			  (STM_USB_EPR_STAT_TX_MASK << STM_USB_EPR_STAT_TX),
			  (STM_USB_EPR_STAT_TX_NAK << STM_USB_EPR_STAT_TX)) |
	       (AO_USB_INT_EP << STM_USB_EPR_EA));
	stm_usb.epr[AO_USB_INT_EPR] = epr;
	sei();
	debug ("writing epr[%d] 0x%08x wrote 0x%08x\n",
	       AO_USB_INT_EPR, epr, stm_usb.epr[AO_USB_INT_EPR]);

	/* Set up the OUT end point */
	ao_usb_bdt[AO_USB_OUT_EPR].single.addr_rx = ao_usb_sram_addr;
	ao_usb_bdt[AO_USB_OUT_EPR].single.count_rx = ((1 << STM_USB_BDT_COUNT_RX_BL_SIZE) |
						      (((AO_USB_OUT_SIZE / 32) - 1) << STM_USB_BDT_COUNT_RX_NUM_BLOCK));
	ao_usb_out_rx_buffer = ao_usb_packet_buffer_addr(ao_usb_sram_addr);
	ao_usb_sram_addr += AO_USB_OUT_SIZE;

	cli();
	epr = stm_usb.epr[AO_USB_OUT_EPR];
	epr = ((0 << STM_USB_EPR_CTR_RX) |
	       (epr & (1 <<  STM_USB_EPR_DTOG_RX)) |
	       set_toggle(epr,
			  (STM_USB_EPR_STAT_RX_MASK << STM_USB_EPR_STAT_RX),
			  (STM_USB_EPR_STAT_RX_VALID << STM_USB_EPR_STAT_RX)) |
	       (STM_USB_EPR_EP_TYPE_CONTROL << STM_USB_EPR_EP_TYPE) |
	       (0 << STM_USB_EPR_EP_KIND) |
	       (0 << STM_USB_EPR_CTR_TX) |
	       (epr & (1 << STM_USB_EPR_DTOG_TX)) |
	       set_toggle(epr,
			  (STM_USB_EPR_STAT_TX_MASK << STM_USB_EPR_STAT_TX),
			  (STM_USB_EPR_STAT_TX_DISABLED << STM_USB_EPR_STAT_TX)) |
	       (AO_USB_OUT_EP << STM_USB_EPR_EA));
	stm_usb.epr[AO_USB_OUT_EPR] = epr;
	sei();
	debug ("writing epr[%d] 0x%08x wrote 0x%08x\n",
	       AO_USB_OUT_EPR, epr, stm_usb.epr[AO_USB_OUT_EPR]);
	
	/* Set up the IN end point */
	ao_usb_bdt[AO_USB_IN_EPR].single.addr_tx = ao_usb_sram_addr;
	ao_usb_bdt[AO_USB_IN_EPR].single.count_tx = 0;
	ao_usb_in_tx_buffer = ao_usb_packet_buffer_addr(ao_usb_sram_addr);
	ao_usb_sram_addr += AO_USB_IN_SIZE;

	cli();
	epr = stm_usb.epr[AO_USB_IN_EPR];
	epr = ((0 << STM_USB_EPR_CTR_RX) |
	       (epr & (1 << STM_USB_EPR_DTOG_RX)) |
	       set_toggle(epr,
			  (STM_USB_EPR_STAT_RX_MASK << STM_USB_EPR_STAT_RX),
			  (STM_USB_EPR_STAT_RX_DISABLED << STM_USB_EPR_STAT_RX)) |
	       (STM_USB_EPR_EP_TYPE_CONTROL << STM_USB_EPR_EP_TYPE) |
	       (0 << STM_USB_EPR_EP_KIND) |
	       (0 << STM_USB_EPR_CTR_TX) |
	       (epr & (1 << STM_USB_EPR_DTOG_TX)) |
	       set_toggle(epr,
			  (STM_USB_EPR_STAT_TX_MASK << STM_USB_EPR_STAT_TX),
			  (STM_USB_EPR_STAT_TX_NAK << STM_USB_EPR_STAT_TX)) |
	       (AO_USB_IN_EP << STM_USB_EPR_EA));
	stm_usb.epr[AO_USB_IN_EPR] = epr;
	sei();
	debug ("writing epr[%d] 0x%08x wrote 0x%08x\n",
	       AO_USB_IN_EPR, epr, stm_usb.epr[AO_USB_IN_EPR]);
	ao_usb_running = 1;
}

static uint16_t	control_count;
static uint16_t	in_count;
static uint16_t	out_count;
static uint16_t	reset_count;

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
			ao_wakeup(&ao_usb_ep0_receive);
			break;
		case AO_USB_OUT_EPR:
			++out_count;
			if (ao_usb_epr_ctr_rx(epr)) {
				ao_usb_out_avail = 1;
				ao_wakeup(&ao_stdin_ready);
			}
			break;
		case AO_USB_IN_EPR:
			++in_count;
			if (ao_usb_epr_ctr_tx(epr)) {
				ao_usb_in_pending = 0;
				ao_wakeup(&ao_usb_in_pending);
			}
			break;
		}
		return;
	}

	if (istr & (1 << STM_USB_ISTR_RESET)) {
		++reset_count;
		stm_usb.istr &= ~(1 << STM_USB_ISTR_RESET);
		ao_usb_ep0_receive |= AO_USB_EP0_GOT_RESET;
		ao_wakeup(&ao_usb_ep0_receive);
	}
}

void
stm_usb_hp_isr(void)
{
	stm_usb_lp_isr();
}

void
stm_usb_fs_wkup(void)
{
	/* USB wakeup, just clear the bit for now */
	stm_usb.istr &= ~(1 << STM_USB_ISTR_WKUP);
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

#if 0
	if (in_pending)
		ueienx_0 = ((1 << RXSTPE) | (1 << RXOUTE) | (1 << TXINE));	/* Enable IN interrupt */
#endif
}

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

static inline void
ao_usb_set_stat_tx(int ep, uint32_t stat_tx) {
	uint32_t	epr_write, epr_old, epr_new, epr_want;

	cli();
	epr_write = epr_old = stm_usb.epr[ep];
	epr_write &= STM_USB_EPR_PRESERVE_MASK;
	epr_write |= STM_USB_EPR_INVARIANT;
	epr_write |= set_toggle(epr_old,
			      STM_USB_EPR_STAT_TX_MASK << STM_USB_EPR_STAT_TX,
			      stat_tx << STM_USB_EPR_STAT_TX);
	stm_usb.epr[ep] = epr_write;
	epr_new = stm_usb.epr[ep];
	sei();
	epr_want = (epr_old & ~(STM_USB_EPR_STAT_TX_MASK << STM_USB_EPR_STAT_TX)) |
		(stat_tx << STM_USB_EPR_STAT_TX);
	if (epr_new != epr_want) {
		debug ("**** set_stat_tx to %x. old %08x want %08x write %08x new %08x\n",
		       stat_tx, epr_old, epr_want, epr_write, epr_new);
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

		debug_data ("Flush EP0 len %d:", this_len);
		ao_usb_write(ao_usb_ep0_in_data, ao_usb_ep0_tx_buffer, 0, this_len);
		debug_data ("\n");
		ao_usb_ep0_in_data += this_len;

		/* Mark the endpoint as TX valid to send the packet */
		ao_usb_bdt[0].single.count_tx = this_len;
		ao_usb_set_stat_tx(0, STM_USB_EPR_STAT_TX_VALID);
		debug ("queue tx. epr 0 now %08x\n", stm_usb.epr[0]);
	}
}

static inline void
ao_usb_set_stat_rx(int ep, uint32_t stat_rx) {
	uint32_t	epr_write, epr_old, epr_new, epr_want;

	cli();
	epr_write = epr_old = stm_usb.epr[ep];
	epr_write &= STM_USB_EPR_PRESERVE_MASK;
	epr_write |= STM_USB_EPR_INVARIANT;
	epr_write |= set_toggle(epr_old,
			      STM_USB_EPR_STAT_RX_MASK << STM_USB_EPR_STAT_RX,
			      stat_rx << STM_USB_EPR_STAT_RX);
	stm_usb.epr[ep] = epr_write;
	epr_new = stm_usb.epr[ep];
	sei();
	epr_want = (epr_old & ~(STM_USB_EPR_STAT_RX_MASK << STM_USB_EPR_STAT_RX)) |
		(stat_rx << STM_USB_EPR_STAT_RX);
	if (epr_new != epr_want) {
		debug ("**** set_stat_rx to %x. old %08x want %08x write %08x new %08x\n",
		       stat_rx, epr_old, epr_want, epr_write, epr_new);
	}
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

void
ao_usb_ep0_queue_byte(uint8_t a)
{
	ao_usb_ep0_in_buf[ao_usb_ep0_in_len++] = a;
}

static void
ao_usb_ep0_setup(void)
{
	/* Pull the setup packet out of the fifo */
	ao_usb_ep0_out_data = (uint8_t *) &ao_usb_setup;
	ao_usb_ep0_out_len = 8;
	ao_usb_ep0_fill();
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
				ao_usb_address = ao_usb_setup.value;
				ao_usb_address_pending = 1;
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
		case SET_LINE_CODING:
			debug ("set line coding\n");
			ao_usb_ep0_out_len = 7;
			ao_usb_ep0_out_data = (uint8_t *) &ao_usb_line_coding;
			break;
		case GET_LINE_CODING:
			debug ("get line coding\n");
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
	uint8_t	intx, udint;

	debug ("usb task started\n");
	ao_usb_ep0_state = AO_USB_EP0_IDLE;
	for (;;) {
		uint8_t	receive;
		ao_arch_critical(
			while (!(receive = ao_usb_ep0_receive))
				ao_sleep(&ao_usb_ep0_receive);
			ao_usb_ep0_receive = 0;
			);
		
		if (receive & AO_USB_EP0_GOT_RESET) {
			debug ("\treset\n");
			ao_usb_set_ep0();
			continue;
		}
		if (receive & AO_USB_EP0_GOT_SETUP) {
			debug ("\tsetup\n");
			ao_usb_ep0_setup();
		}
		if (receive & AO_USB_EP0_GOT_RX_DATA) {
			debug ("\tgot rx data\n");
			ao_usb_ep0_fill();
			ao_usb_ep0_set_in_pending(1);
		}
		if (receive & AO_USB_EP0_GOT_TX_ACK) {
			debug ("\tgot tx ack\n");
			ao_usb_ep0_flush();
			if (ao_usb_address_pending) {
				ao_usb_set_address(ao_usb_address);
				ao_usb_set_configuration();
			}
		}
	}
}

/* Queue the current IN buffer for transmission */
static void
ao_usb_in_send(void)
{
	debug ("send %d\n", ao_usb_tx_count);
	ao_usb_write(ao_usb_tx_buffer, ao_usb_in_tx_buffer, 0, ao_usb_tx_count);
	ao_usb_bdt[AO_USB_IN_EPR].single.count_tx = ao_usb_tx_count;
	ao_usb_set_stat_tx(AO_USB_IN_EPR, STM_USB_EPR_STAT_TX_VALID);
	ao_usb_in_pending = 1;
	ao_usb_tx_count = 0;
}

/* Wait for a free IN buffer */
static void
ao_usb_in_wait(void)
{
	for (;;) {
		/* Check if the current buffer is writable */
		if (ao_usb_tx_count < AO_USB_IN_SIZE)
			break;

		cli();
		/* Wait for an IN buffer to be ready */
		while (ao_usb_in_pending)
			ao_sleep(&ao_usb_in_pending);
		sei();
	}
}

void
ao_usb_flush(void) __critical
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
	if (!ao_usb_in_flushed) {
		ao_usb_in_flushed = 1;
		cli();
		/* Wait for an IN buffer to be ready */
		while (ao_usb_in_pending)
			ao_sleep(&ao_usb_in_pending);
		sei();
		ao_usb_in_send();
	}
}

void
ao_usb_putchar(char c) __critical __reentrant
{
	if (!ao_usb_running)
		return;

	ao_usb_in_wait();

	ao_usb_tx_buffer[ao_usb_tx_count++] = (uint8_t) c;

	/* Send the packet when full */
	if (ao_usb_tx_count == AO_USB_IN_SIZE)
		ao_usb_in_send();
	ao_usb_in_flushed = 0;
}

static void
ao_usb_out_recv(void)
{
	ao_usb_out_avail = 0;

	ao_usb_rx_count = ao_usb_bdt[AO_USB_OUT_EPR].single.count_rx & STM_USB_BDT_COUNT_RX_COUNT_RX_MASK;

	debug ("recv %d\n", ao_usb_rx_count);
	debug_data("Fill OUT len %d:", ao_usb_rx_count);
	ao_usb_read(ao_usb_rx_buffer, ao_usb_out_rx_buffer, 0, ao_usb_rx_count);
	debug_data("\n");
	ao_usb_rx_pos = 0;

	/* ACK the packet */
	ao_usb_set_stat_rx(AO_USB_OUT_EPR, STM_USB_EPR_STAT_RX_VALID);
}

static char
_ao_usb_pollchar(void)
{
	char c;

	if (!ao_usb_running)
		return AO_READ_AGAIN;

	for (;;) {
		if (ao_usb_rx_pos != ao_usb_rx_count)
			break;

		/* Check to see if a packet has arrived */
		if (!ao_usb_out_avail)
			return AO_READ_AGAIN;
		ao_usb_out_recv();
	}

	/* Pull a character out of the fifo */
	c = ao_usb_rx_buffer[ao_usb_rx_pos++];
	return c;
}

char
ao_usb_pollchar(void)
{
	char	c;
	cli();
	c = _ao_usb_pollchar();
	sei();
	return c;
}

char
ao_usb_getchar(void) __critical
{
	char	c;

	cli();
	while ((c = _ao_usb_pollchar()) == AO_READ_AGAIN)
		ao_sleep(&ao_stdin_ready);
	sei();
	return c;
}

void
ao_usb_disable(void)
{
	stm_usb.cntr = (1 << STM_USB_CNTR_FRES);
	stm_usb.istr = 0;

	/* Disable USB pull-up */
	stm_syscfg.pmc &= ~(1 << STM_SYSCFG_PMC_USB_PU);

	/* Switch off the device */
	stm_usb.cntr = (1 << STM_USB_CNTR_PDWN) | (1 << STM_USB_CNTR_FRES);

	/* Disable the interface */
	stm_rcc.apb1enr &+ ~(1 << STM_RCC_APB1ENR_USBEN);
}

void
ao_usb_enable(void)
{
	uint16_t	tick;

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

	/* Enable USB pull-up */
	stm_syscfg.pmc |= (1 << STM_SYSCFG_PMC_USB_PU);
}

#if USB_DEBUG
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

static void
ao_usb_irq(void)
{
	printf ("control: %d out: %d in: %d reset: %d\n",
		control_count, out_count, in_count, reset_count);
}

__code struct ao_cmds ao_usb_cmds[] = {
	{ ao_usb_irq, "I\0Show USB interrupt counts" },
	{ 0, NULL }
};

void
ao_usb_init(void)
{
	ao_usb_enable();

	debug ("ao_usb_init\n");
	ao_add_task(&ao_usb_task, ao_usb_ep0, "usb");
#if USB_DEBUG
	ao_add_task(&ao_usb_echo_task, ao_usb_echo, "usb echo");
#endif
	ao_cmd_register(&ao_usb_cmds[0]);
#if !USB_DEBUG
	ao_add_stdio(ao_usb_pollchar, ao_usb_putchar, ao_usb_flush);
#endif
}

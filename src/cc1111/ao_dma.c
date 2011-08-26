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

#define NUM_DMA	5

/*
 * The config address for DMA0 is programmed
 * separately from that of DMA1-4, but for simplicity,
 * we make them all contiguous.
 */

static __xdata struct cc_dma_channel ao_dma_config[NUM_DMA];
static __xdata uint8_t * __xdata ao_dma_done[NUM_DMA];
static __data uint8_t ao_next_dma;

uint8_t
ao_dma_alloc(__xdata uint8_t *done)
{
	uint8_t id;

	if (ao_next_dma == NUM_DMA)
		ao_panic(AO_PANIC_DMA);
	id = ao_next_dma++;
	ao_dma_done[id] = done;

	/* When the first dma object is allocated, set up the DMA
	 * controller
	 */
	if (id == 0) {
		DMAIRQ = 0;
		DMAIF = 0;
		IEN1 |= IEN1_DMAIE;
	}

	return id;
}

void
ao_dma_set_transfer(uint8_t id,
		    void __xdata *srcaddr,
		    void __xdata *dstaddr,
		    uint16_t count,
		    uint8_t cfg0,
		    uint8_t cfg1)
{
	if (DMAARM & (1 << id))
		ao_panic(AO_PANIC_DMA);
	ao_dma_config[id].src_high = ((uint16_t) srcaddr) >> 8;
	ao_dma_config[id].src_low = ((uint16_t) srcaddr);
	ao_dma_config[id].dst_high = ((uint16_t) dstaddr) >> 8;
	ao_dma_config[id].dst_low = ((uint16_t) dstaddr);
	ao_dma_config[id].len_high = count >> 8;
	ao_dma_config[id].len_low = count;
	ao_dma_config[id].cfg0 = cfg0;
	ao_dma_config[id].cfg1 = cfg1 | DMA_CFG1_IRQMASK;
	if (id == 0) {
		DMA0CFGH = ((uint16_t) (&ao_dma_config[0])) >> 8;
		DMA0CFGL = ((uint16_t) (&ao_dma_config[0]));
	} else {
		DMA1CFGH = ((uint16_t) (&ao_dma_config[1])) >> 8;
		DMA1CFGL = ((uint16_t) (&ao_dma_config[1]));
	}
}

#define nop()	_asm nop _endasm;

void
ao_dma_start(uint8_t id)
{
	uint8_t	mask = (1 << id);
	DMAIRQ &= ~mask;
	DMAARM = 0x80 | mask;
	nop(); nop(); nop(); nop();
	nop(); nop(); nop(); nop();
	*(ao_dma_done[id]) = 0;
	DMAARM = mask;
	nop(); nop(); nop(); nop();
	nop(); nop(); nop(); nop();
	nop();
}

void
ao_dma_trigger(uint8_t id)
{
	DMAREQ |= (1 << id);
}

void
ao_dma_abort(uint8_t id)
{
	uint8_t	mask = (1 << id);
	DMAARM = 0x80 | mask;
	DMAIRQ &= ~mask;
}

void
ao_dma_isr(void) __interrupt 8
{
	uint8_t id, mask;

	/* Find the first DMA channel which is done */
	mask = 1;
	for (id = 0; id < ao_next_dma; id++) {
		if (DMAIRQ & mask) {
			/* Clear CPU interrupt flag */
			DMAIF = 0;
			/* Clear the completed ID */
			DMAIRQ = ~mask;
			*(ao_dma_done[id]) = 1;
			ao_wakeup(ao_dma_done[id]);
			break;
		}
		mask <<= 1;
	}
}

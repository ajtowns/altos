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

#if !HAS_AES
#error Must define HAS_AES 1
#endif

__xdata uint8_t ao_aes_mutex;
__xdata uint8_t	ao_aes_done;
__xdata uint8_t	ao_aes_dma_in, ao_aes_dma_out;
__xdata uint8_t ao_aes_dma_in_done, ao_aes_dma_out_done;
__pdata enum ao_aes_mode ao_aes_current_mode;

void
ao_aes_isr(void) __interrupt 4
{
	S0CON = 0;
	if (ENCCCS & ENCCCS_RDY) {
		ao_aes_done = 1;
		ao_wakeup(&ao_aes_done);
	}
}

void
ao_aes_set_mode(enum ao_aes_mode mode)
{
	ao_aes_current_mode = mode;
}

void
ao_aes_set_key(__xdata uint8_t *in)
{
	ao_dma_set_transfer(ao_aes_dma_in,
			    in,
			    &ENCDIXADDR,
			    AO_AES_LEN,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_ENC_DW,
			    DMA_CFG1_SRCINC_1 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_LOW);
	ao_dma_start(ao_aes_dma_in);
	ao_aes_done = 0;
	ENCCCS = ENCCCS_MODE_CBC_MAC |
		ENCCCS_CMD_LOAD_KEY;
	ENCCCS |= ENCCCS_START;
	__critical while (!ao_aes_done)
		ao_sleep(&ao_aes_done);
}

void
ao_aes_zero_iv(void)
{
	uint8_t	b;

	ENCCCS = ENCCCS_MODE_CBC_MAC | ENCCCS_CMD_LOAD_IV | ENCCCS_START;
	for (b = 0; b < AO_AES_LEN; b++)
		ENCDI = 0;
}

void
ao_aes_run(__xdata uint8_t *in,
	   __xdata uint8_t *out)
{
	uint8_t	b;
	if (in) {
		ao_dma_set_transfer(ao_aes_dma_in,
				    in,
				    &ENCDIXADDR,
				    AO_AES_LEN,
				    DMA_CFG0_WORDSIZE_8 |
				    DMA_CFG0_TMODE_SINGLE |
				    DMA_CFG0_TRIGGER_ENC_DW,
				    DMA_CFG1_SRCINC_1 |
				    DMA_CFG1_DESTINC_0 |
				    DMA_CFG1_PRIORITY_LOW);
	}
	if (out) {
		ao_dma_set_transfer(ao_aes_dma_out,
				    &ENCDOXADDR,
				    out,
				    AO_AES_LEN,
				    DMA_CFG0_WORDSIZE_8 |
				    DMA_CFG0_TMODE_SINGLE |
				    DMA_CFG0_TRIGGER_ENC_UP,
				    DMA_CFG1_SRCINC_0 |
				    DMA_CFG1_DESTINC_1 |
				    DMA_CFG1_PRIORITY_LOW);
	}
	switch (ao_aes_current_mode) {
	case ao_aes_mode_cbc_mac:
		if (out)
			b = (ENCCCS_MODE_CBC |
			     ENCCCS_CMD_ENCRYPT);
		else
			b = (ENCCCS_MODE_CBC_MAC |
			     ENCCCS_CMD_ENCRYPT);
		break;
	default:
		return;
	}
	ao_aes_done = 0;
	if (in)
		ao_dma_start(ao_aes_dma_in);
	if (out)
		ao_dma_start(ao_aes_dma_out);
	ENCCCS = b;
	ENCCCS |= ENCCCS_START;
	if (out) {
		__critical while (!ao_aes_dma_out_done)
			ao_sleep(&ao_aes_dma_out_done);
	} else {
		__critical while (!ao_aes_done)
			ao_sleep(&ao_aes_done);
	}
}

void
ao_aes_init(void)
{
#if DMA_SHARE_AES_RADIO
	ao_aes_dma_in = ao_radio_dma;
#else
	ao_aes_dma_in = ao_dma_alloc(&ao_aes_dma_in_done);
#endif
	ao_aes_dma_out = ao_dma_alloc(&ao_aes_dma_out_done);
	S0CON = 0;
	ENCIE = 1;
}

/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

#include <ao.h>

static uint8_t		ao_spi_mutex[LPC_NUM_SPI];

static struct lpc_ssp * const ao_lpc_ssp[LPC_NUM_SPI] = { &lpc_ssp0, &lpc_ssp1 };

#define tx_busy(lpc_ssp) (lpc_ssp->sr & ((1 << LPC_SSP_SR_BSY) | (1 << LPC_SSP_SR_TNF))) != (1 << LPC_SSP_SR_TNF)
#define rx_busy(lpc_ssp) (lpc_ssp->sr & ((1 << LPC_SSP_SR_BSY) | (1 << LPC_SSP_SR_RNE))) != (1 << LPC_SSP_SR_RNE)

#define spi_loop(len, put, get) do {					\
		while (len--) {						\
			/* Wait for space in the fifo */		\
			while (tx_busy(lpc_ssp))			\
				;					\
									\
			/* send a byte */				\
			lpc_ssp->dr = put;				\
									\
			/* Wait for byte to appear in the fifo */	\
			while (rx_busy(lpc_ssp))			\
				;					\
									\
			/* recv a byte */				\
			get lpc_ssp->dr;				\
		}							\
	} while (0)

void
ao_spi_send(void *block, uint16_t len, uint8_t id)
{
	uint8_t	*b = block;
	struct lpc_ssp *lpc_ssp = ao_lpc_ssp[id];

	spi_loop(len, *b++, (void));
}

void
ao_spi_send_fixed(uint8_t value, uint16_t len, uint8_t id)
{
	struct lpc_ssp *lpc_ssp = ao_lpc_ssp[id];

	spi_loop(len, value, (void));
}

void
ao_spi_recv(void *block, uint16_t len, uint8_t id)
{
	uint8_t	*b = block;
	struct lpc_ssp *lpc_ssp = ao_lpc_ssp[id];

	spi_loop(len, 0xff, *b++ =);
}

void
ao_spi_duplex(void *out, void *in, uint16_t len, uint8_t id)
{
	uint8_t	*o = out;
	uint8_t	*i = in;
	struct lpc_ssp *lpc_ssp = ao_lpc_ssp[id];

	spi_loop(len, *o++, *i++ =);
}

void
ao_spi_get(uint8_t id, uint32_t speed)
{
	struct lpc_ssp	*lpc_ssp = ao_lpc_ssp[id];

	ao_mutex_get(&ao_spi_mutex[id]);
	
	/* Set the clock prescale */
	lpc_ssp->cpsr = speed;
}

void
ao_spi_put(uint8_t id)
{
	ao_mutex_put(&ao_spi_mutex[id]);
}

static void
ao_spi_channel_init(uint8_t id)
{
	struct lpc_ssp	*lpc_ssp = ao_lpc_ssp[id];
	uint8_t	d;

	lpc_ssp->cr0 = ((LPC_SSP_CR0_DSS_8 << LPC_SSP_CR0_DSS) |
			(LPC_SSP_CR0_FRF_SPI << LPC_SSP_CR0_FRF) |
			(0 << LPC_SSP_CR0_CPOL) |
			(0 << LPC_SSP_CR0_CPHA) |
			(0 << LPC_SSP_CR0_SCR));

	/* Enable the device */
	lpc_ssp->cr1 = ((0 << LPC_SSP_CR1_LBM) |
			(1 << LPC_SSP_CR1_SSE) |
			(LPC_SSP_CR1_MS_MASTER << LPC_SSP_CR1_MS) |
			(0 << LPC_SSP_CR1_SOD));

	/* Drain the receive fifo */
	for (d = 0; d < LPC_SSP_FIFOSIZE; d++)
		(void) lpc_ssp->dr;
}

void
ao_spi_init(void)
{
#if HAS_SPI_0
	/* Configure pins */
#if SPI_SCK0_P0_6
	lpc_ioconf.pio0_6 = ao_lpc_alternate(LPC_IOCONF_FUNC_PIO0_6_SCK0);
#define HAS_SCK0
#endif
#if SPI_SCK0_P0_10
	lpc_ioconf.pio0_10 = ao_lpc_alternate(LPC_IOCONF_FUNC_PIO0_10_SCK0);
#define HAS_SCK0
#endif
#if SPI_SCK0_P1_29
	lpc_ioconf.pio1_29 = ao_lpc_alternate(LPC_IOCONF_FUNC_PIO1_29_SCK0);
#define HAS_SCK0
#endif
#ifndef HAS_SCK0
#error "No pin specified for SCK0"
#endif
	lpc_ioconf.pio0_8 = ao_lpc_alternate(LPC_IOCONF_FUNC_MISO0);
	lpc_ioconf.pio0_9 = ao_lpc_alternate(LPC_IOCONF_FUNC_MOSI0);

	/* Enable the device */
	lpc_scb.sysahbclkctrl |= (1 << LPC_SCB_SYSAHBCLKCTRL_SSP0);

	/* Turn on the clock */
	lpc_scb.ssp0clkdiv = 1;

	/* Reset the device */
	lpc_scb.presetctrl &= ~(1 << LPC_SCB_PRESETCTRL_SSP0_RST_N);
	lpc_scb.presetctrl |= (1 << LPC_SCB_PRESETCTRL_SSP0_RST_N);
	ao_spi_channel_init(0);
#endif			   

#if HAS_SPI_1

#if SPI_SCK1_P1_15
	lpc_ioconf.pio1_15 = ao_lpc_alternate(LPC_IOCONF_FUNC_PIO1_15_SCK1);
#define HAS_SCK1
#endif
#if SPI_SCK1_P1_20
	lpc_ioconf.pio1_20 = ao_lpc_alternate(LPC_IOCONF_FUNC_PIO1_20_SCK1);
#define HAS_SCK1
#endif
#ifndef HAS_SCK1
#error "No pin specified for SCK1"
#endif

#if SPI_MISO1_P0_22
	lpc_ioconf.pio0_22 = ao_lpc_alternate(LPC_IOCONF_FUNC_PIO0_22_MISO1);
#define HAS_MISO1
#endif
#if SPI_MISO1_P1_21
	lpc_ioconf.pio1_21 = ao_lpc_alternate(LPC_IOCONF_FUNC_PIO1_21_MISO1);
#define HAS_MISO1
#endif
#ifndef HAS_MISO1
#error "No pin specified for MISO1"
#endif

#if SPI_MOSI1_P0_21
	lpc_ioconf.pio0_21 = ao_lpc_alternate(LPC_IOCONF_FUNC_PIO0_21_MOSI1);
#define HAS_MOSI1
#endif
#if SPI_MOSI1_P1_22
	lpc_ioconf.pio1_22 = ao_lpc_alternate(LPC_IOCONF_FUNC_PIO1_22_MOSI1);
#define HAS_MOSI1
#endif
#ifndef HAS_MOSI1
#error "No pin specified for MOSI1"
#endif
		
	/* Enable the device */
	lpc_scb.sysahbclkctrl |= (1 << LPC_SCB_SYSAHBCLKCTRL_SSP1);

	/* Turn on the clock */
	lpc_scb.ssp1clkdiv = 1;

	/* Reset the device */
	lpc_scb.presetctrl &= ~(1 << LPC_SCB_PRESETCTRL_SSP1_RST_N);
	lpc_scb.presetctrl |= (1 << LPC_SCB_PRESETCTRL_SSP1_RST_N);
	ao_spi_channel_init(1);
#endif /* HAS_SPI_1 */
}

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

#ifndef _AO_ARCH_FUNCS_H_
#define _AO_ARCH_FUNCS_H_

/* ao_spi_stm.c
 */
extern uint8_t	ao_spi_mutex[STM_NUM_SPI];

void
ao_spi_get(uint8_t spi_index);

void
ao_spi_put(uint8_t spi_index);

void
ao_spi_send(void *block, uint16_t len, uint8_t spi_index);

void
ao_spi_recv(void *block, uint16_t len, uint8_t spi_index);

void
ao_spi_init(void);

#define ao_spi_get_mask(reg,mask,bus) do {		\
		ao_spi_get(bus);			\
		(reg).bsrr = ((uint32_t) mask) << 16;	\
	} while (0)

#define ao_spi_put_mask(reg,mask,bus) do {	\
		(reg).bsrr = mask;		\
		ao_spi_put(bus);		\
	} while (0)

#define ao_stm_enable_port(port) do {					\
		if (&(port) == &stm_gpioa)				\
			stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOAEN); \
		else if (&(port) == &stm_gpiob)				\
			stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOBEN); \
		else if (&(port) == &stm_gpioc)				\
			stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOCEN); \
		else if (&(port) == &stm_gpiod)				\
			stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIODEN); \
		else if (&(port) == &stm_gpioe)				\
			stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOEEN); \
	} while (0)


#define ao_stm_enable_cs(port,bit) do {				\
		stm_gpio_set(&(port), bit, 1);			\
		stm_moder_set(&(port), bit, STM_MODER_OUTPUT);	\
	} while (0)

#define ao_spi_init_cs(port, mask) do {				\
		ao_stm_enable_port(port);			\
		if (mask & 0x0001) ao_stm_enable_cs(port, 0);	\
		if (mask & 0x0002) ao_stm_enable_cs(port, 1);	\
		if (mask & 0x0004) ao_stm_enable_cs(port, 2);	\
		if (mask & 0x0008) ao_stm_enable_cs(port, 3);	\
		if (mask & 0x0010) ao_stm_enable_cs(port, 4);	\
		if (mask & 0x0020) ao_stm_enable_cs(port, 5);	\
		if (mask & 0x0040) ao_stm_enable_cs(port, 6);	\
		if (mask & 0x0080) ao_stm_enable_cs(port, 7);	\
		if (mask & 0x0100) ao_stm_enable_cs(port, 8);	\
		if (mask & 0x0200) ao_stm_enable_cs(port, 9);	\
		if (mask & 0x0400) ao_stm_enable_cs(port, 10);	\
		if (mask & 0x0800) ao_stm_enable_cs(port, 11);	\
		if (mask & 0x1000) ao_stm_enable_cs(port, 12);	\
		if (mask & 0x2000) ao_stm_enable_cs(port, 13);	\
		if (mask & 0x4000) ao_stm_enable_cs(port, 14);	\
		if (mask & 0x8000) ao_stm_enable_cs(port, 15);	\
	} while (0)

/* ao_dma_stm.c
 */

extern uint8_t ao_dma_done[STM_NUM_DMA];

void
ao_dma_set_transfer(uint8_t 		index,
		    volatile void	*peripheral,
		    void		*memory,
		    uint16_t		count,
		    uint32_t		ccr);

void
ao_dma_set_isr(uint8_t index, void (*isr)(void));

void
ao_dma_start(uint8_t index);

void
ao_dma_done_transfer(uint8_t index);

void
ao_dma_abort(uint8_t index);

void
ao_dma_alloc(uint8_t index);

void
ao_dma_init(void);

/* ao_i2c_stm.c */

void
ao_i2c_get(uint8_t i2c_index);

uint8_t
ao_i2c_start(uint8_t i2c_index, uint16_t address);

void
ao_i2c_put(uint8_t i2c_index);

void
ao_i2c_send(void *block, uint16_t len, uint8_t i2c_index);

void
ao_i2c_recv(void *block, uint16_t len, uint8_t i2c_index);

void
ao_i2c_init(void);

#endif /* _AO_ARCH_FUNCS_H_ */
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

struct ao_task demo_task;

static inline int min(int a, int b) { return a < b ? a : b; }

void
ao_demo(void)
{
	char	message[] = "Hello, Mike & Bdale --- ";
	char	part[7];
	int	i = 0;
	int	len = sizeof(message) - 1;
	int	first, second;

	part[6] = '\0';
	for (;;) {
		ao_delay(AO_MS_TO_TICKS(150));
		first = min(6, len - i);
		second = 6 - first;
		memcpy(part, message + i, first);
		memcpy(part + first, message, second);
		ao_lcd_font_string(part);
		if (++i >= len)
			i = 0;
	}
}

void _close() { }
void _sbrk() { }
void _isatty() { }
void _lseek() { }
void _exit () { }
void _read () { }
void _fstat() { }

#define AO_DMA_TEST_INDEX	STM_DMA_INDEX(4)

static void
ao_dma_test(void) {
	static char	src[20] = "hello, world";
	static char	dst[20];
	
	dst[0] = '\0';
	ao_dma_set_transfer(AO_DMA_TEST_INDEX, dst, src, 13,
			    (1 << STM_DMA_CCR_MEM2MEM) |
			    (STM_DMA_CCR_PL_LOW << STM_DMA_CCR_PL) |
			    (STM_DMA_CCR_MSIZE_8 << STM_DMA_CCR_MSIZE) |
			    (STM_DMA_CCR_PSIZE_8 << STM_DMA_CCR_PSIZE) |
			    (1 << STM_DMA_CCR_MINC) |
			    (1 << STM_DMA_CCR_PINC) |
			    (0 << STM_DMA_CCR_CIRC) |
			    (STM_DMA_CCR_DIR_MEM_TO_PER << STM_DMA_CCR_DIR));
	ao_dma_start(AO_DMA_TEST_INDEX);
	ao_arch_critical(
		while (!ao_dma_done[AO_DMA_TEST_INDEX])
			ao_sleep(&ao_dma_done[AO_DMA_TEST_INDEX]);
		);
	ao_dma_done_transfer(AO_DMA_TEST_INDEX);
	printf ("copied %s\n", dst);
}

static void
ao_spi_write(void) {
	unsigned char	data[] = { 0x55, 0xaa, 0xff, 0x00 };
	int i;

	for (i = 0; i < 10; i++) {
		ao_spi_get(0);
		stm_gpio_set(&stm_gpioc, 12, 0);
		ao_spi_send(data, 4, 0);
		stm_gpio_set(&stm_gpioc, 12, 1);
		ao_spi_put(0);
		printf(".");
		flush();
		ao_delay(100);
	}
}

static void
ao_spi_read(void) {
	unsigned char	data[4];
	int i;

	for (i = 0; i < 10; i++) {
		ao_spi_get(0);
		stm_gpio_set(&stm_gpioc, 12, 0);
		ao_spi_recv(data, 4, 0);
		stm_gpio_set(&stm_gpioc, 12, 1);
		ao_spi_put(0);
		printf(".");
		flush();
		ao_delay(100);
	}
}

__code struct ao_cmds ao_demo_cmds[] = {
	{ ao_dma_test,	"D\0DMA test" },
	{ ao_spi_write, "W\0SPI write" },
	{ ao_spi_read, "R\0SPI read" },
	{ 0, NULL }
};

int
main(void)
{
	ao_clock_init();
	
	ao_serial_init();
	ao_timer_init();
	ao_dma_init();
	ao_cmd_init();
//	ao_lcd_stm_init();
//	ao_lcd_font_init();
	ao_spi_init();

	ao_timer_set_adc_interval(100);

	ao_adc_init();

	ao_cmd_register(&ao_demo_cmds[0]);
	
	ao_start_scheduler();
	return 0;
}

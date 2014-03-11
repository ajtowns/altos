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
#include <ao_exti.h>
#include <ao_event.h>
#include <ao_quadrature.h>
#include <ao_button.h>
#include <ao_boot.h>

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
		ao_spi_get(0, AO_SPI_SPEED_FAST);
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
		ao_spi_get(0, AO_SPI_SPEED_FAST);
		stm_gpio_set(&stm_gpioc, 12, 0);
		ao_spi_recv(data, 4, 0);
		stm_gpio_set(&stm_gpioc, 12, 1);
		ao_spi_put(0);
		printf(".");
		flush();
		ao_delay(100);
	}
}

static void
ao_i2c_write(void) {
	unsigned char	data[] = { 0x55, 0xaa, 0xff, 0x00 };
	int i;

	for (i = 0; i < 10; i++) {
		ao_i2c_get(0);
		if (ao_i2c_start(0, 0x55))
			ao_i2c_send(data, 4, 0, TRUE);
		else {
			printf ("i2c start failed\n");
			ao_i2c_put(0);
			break;
		}
		ao_i2c_put(0);
		printf(".");
		flush();
		ao_delay(100);
	}
}

static void
ao_temp (void)
{
	struct ao_data	packet;
	int temp;

	ao_data_get(&packet);

	/*
	 * r = (110 - 25) / (ts_cal_hot - ts_cal_cold)
	 * 25 + (110 - 25) * (temp - ts_cal_cold) / (ts_cal_hot - ts_cal_cold)
	 */
	temp = 25 + (110 - 25) * (packet.adc.temp - stm_temp_cal.ts_cal_cold) / (stm_temp_cal.ts_cal_hot - stm_temp_cal.ts_cal_cold);
	printf ("temp: %d\n", temp);
}

#if 0
static void
ao_event(void)
{
	struct ao_event	event;

	for (;;) {
		flush();
		ao_event_get(&event);
		printf ("type %1d unit %1d tick %5u value %ld\n",
			event.type, event.unit, event.tick, event.value);
		if (event.value == 100)
			break;
	}

}
#endif

static uint8_t ao_blinking = 0;

static void
ao_blink(void)
{
	for (;;) {
		while (!ao_blinking)
			ao_sleep(&ao_blinking);
		while (ao_blinking) {
			ao_led_toggle(AO_LED_BLUE|AO_LED_GREEN);
			ao_delay(AO_MS_TO_TICKS(500));
		}
	}
}

static struct ao_task ao_blink_task;

static void
ao_blink_toggle(void)
{
	ao_blinking = !ao_blinking;
	ao_wakeup(&ao_blinking);
}


__code struct ao_cmds ao_demo_cmds[] = {
	{ ao_dma_test,	"D\0DMA test" },
	{ ao_spi_write, "W\0SPI write" },
	{ ao_spi_read, "R\0SPI read" },
	{ ao_i2c_write, "i\0I2C write" },
	{ ao_temp, "t\0Show temp" },
	{ ao_blink_toggle, "b\0Toggle LED blinking" },
/*	{ ao_event, "e\0Monitor event queue" }, */
	{ 0, NULL }
};

int
main(void)
{
	ao_clock_init();
	
	ao_task_init();

	ao_led_init(LEDS_AVAILABLE);
	ao_led_on(AO_LED_GREEN);
	ao_led_off(AO_LED_BLUE);
	ao_timer_init();
	ao_dma_init();
	ao_cmd_init();
//	ao_lcd_stm_init();
//	ao_lcd_font_init();
//	ao_spi_init();
//	ao_i2c_init();
//	ao_exti_init();
//	ao_quadrature_init();
//	ao_button_init();

//	ao_timer_set_adc_interval(100);

//	ao_adc_init();
	ao_usb_init();

	ao_add_task(&ao_blink_task, ao_blink, "blink");
	ao_cmd_register(&ao_demo_cmds[0]);
	
	ao_start_scheduler();
	return 0;
}

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

struct ao_task __xdata blink_0_task;
struct ao_task __xdata blink_1_task;
struct ao_task __xdata wakeup_task;
struct ao_task __xdata beep_task;
struct ao_task __xdata echo_task;

void delay(int n) __reentrant
{
	uint8_t	j = 0;
	while (--n)
		while (--j)
			ao_yield();
}

static __xdata uint8_t blink_chan;

void
blink_0(void)
{
	uint8_t	b = 0;
	for (;;) {
		b = 1 - b;
		if (b)
			ao_led_on(AO_LED_GREEN);
		else
			ao_led_off(AO_LED_GREEN);
		ao_sleep(&blink_chan);
	}
}

void
blink_1(void)
{
	static __xdata struct ao_adc adc;

	for (;;) {
		ao_sleep(&ao_adc_head);
		ao_adc_get(&adc);
		if (adc.accel < 15900)
			ao_led_on(AO_LED_RED);
		else
			ao_led_off(AO_LED_RED);
	}
}

void
wakeup(void)
{
	for (;;) {
		ao_delay(AO_MS_TO_TICKS(100));
		ao_wakeup(&blink_chan);
	}
}

void
beep(void)
{
	static __xdata struct ao_adc adc;

	for (;;) {
		ao_delay(AO_SEC_TO_TICKS(1));
		ao_adc_get(&adc);
		if (adc.temp > 7400)
			ao_beep_for(AO_BEEP_LOW, AO_MS_TO_TICKS(50));
	}
}

void
echo(void)
{
	char	c;
	for (;;) {
		ao_usb_flush();
		c = ao_usb_getchar();
		ao_usb_putchar(c);
		if (c == '\r')
			ao_usb_putchar('\n');
	}
}

void
main(void)
{
	ao_clock_init();

//	ao_add_task(&blink_0_task, blink_0);
//	ao_add_task(&blink_1_task, blink_1);
//	ao_add_task(&wakeup_task, wakeup);
//	ao_add_task(&beep_task, beep);
	ao_add_task(&echo_task, echo);
	ao_timer_init();
	ao_adc_init();
	ao_beep_init();
	ao_led_init();
	ao_usb_init();

	ao_start_scheduler();
}

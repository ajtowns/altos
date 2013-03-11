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

#include <ao.h>
#include "stm32l.h"
#include <string.h>

extern void main(void);
extern char __stack__;
extern char __text_start__, __text_end__;
extern char __data_start__, __data_end__;
extern char __bss_start__, __bss_end__;

/* Interrupt functions */

void stm_halt_isr(void)
{
	ao_panic(AO_PANIC_CRASH);
}

void stm_ignore_isr(void)
{
}

const void *stm_interrupt_vector[];

#define BOOT_FETCH(o)	(*((uint32_t *) (AO_BOOT_APPLICATION_BASE + (o))))

#ifdef AO_BOOT_APPLICATION_PIN
#include <ao_exti.h>

#define AO_BOOT_APPLICATION		0x5a5aa5a5
#define AO_BOOT_APPLICATION_CHECK	0xc3c33c3c

static uint32_t	ao_boot_application;
static uint32_t	ao_boot_application_check;

static void
ao_boot_chain(void) {
	uint32_t	sp;
	uint32_t	pc;

	sp = BOOT_FETCH(0);
	pc = BOOT_FETCH(4);
	asm ("mov sp, %0" : : "r" (sp));
	asm ("mov lr, %0" : : "r" (pc));
	asm ("bx lr");
}

void
ao_reboot_application(void)
{
	ao_boot_application = AO_BOOT_APPLICATION;
	ao_boot_application_check = AO_BOOT_APPLICATION_CHECK;
	ao_arch_reboot();
}

#endif

void start(void) {
#ifdef AO_BOOT_APPLICATION_PIN
	uint16_t v;

	if (ao_boot_application == AO_BOOT_APPLICATION &&
	    ao_boot_application_check == AO_BOOT_APPLICATION_CHECK) {
		ao_boot_application = 0;
		ao_boot_application_check = 0;
		ao_boot_chain();
	}
	/* Enable power interface clock */
	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_PWREN);
	
	/* Enable the input pin */
	ao_enable_input(&AO_BOOT_APPLICATION_GPIO, AO_BOOT_APPLICATION_PIN,
			AO_BOOT_APPLICATION_MODE);

	/* Read the value */
	v = stm_gpio_get(&AO_BOOT_APPLICATION_GPIO, AO_BOOT_APPLICATION_PIN);

	/* Reset the chip to turn off the port and the power interface clock */
	ao_gpio_set_mode(&AO_BOOT_APPLICATION_GPIO, AO_BOOT_APPLICATION_PIN, 0);
	ao_disable_port(&AO_BOOT_APPLICATION_GPIO);
	stm_rcc.apb1enr &= ~(1 << STM_RCC_APB1ENR_PWREN);
	if (v == AO_BOOT_APPLICATION_VALUE)
		ao_boot_chain();
#endif

	/* Set interrupt vector table offset */
	stm_nvic.vto = (uint32_t) &stm_interrupt_vector;
	memcpy(&__data_start__, &__text_end__, &__data_end__ - &__data_start__);
	memset(&__bss_start__, '\0', &__bss_end__ - &__bss_start__);
	main();
}

#define STRINGIFY(x) #x

#define isr(name) \
	void __attribute__ ((weak)) stm_ ## name ## _isr(void); \
	_Pragma(STRINGIFY(weak stm_ ## name ## _isr = stm_ignore_isr))

#define isr_halt(name) \
	void __attribute__ ((weak)) stm_ ## name ## _isr(void); \
	_Pragma(STRINGIFY(weak stm_ ## name ## _isr = stm_halt_isr))

isr(nmi)
isr_halt(hardfault)
isr_halt(memmanage)
isr_halt(busfault)
isr_halt(usagefault)
isr(svc)
isr(debugmon)
isr(pendsv)
isr(systick)
isr(wwdg)
isr(pvd)
isr(tamper_stamp)
isr(rtc_wkup)
isr(flash)
isr(rcc)
isr(exti0)
isr(exti1)
isr(exti2)
isr(exti3)
isr(exti4)
isr(dma1_channel1)
isr(dma1_channel2)
isr(dma1_channel3)
isr(dma1_channel4)
isr(dma1_channel5)
isr(dma1_channel6)
isr(dma1_channel7)
isr(adc1)
isr(usb_hp)
isr(usb_lp)
isr(dac)
isr(comp)
isr(exti9_5)
isr(lcd)
isr(tim9)
isr(tim10)
isr(tim11)
isr(tim2)
isr(tim3)
isr(tim4)
isr(i2c1_ev)
isr(i2c1_er)
isr(i2c2_ev)
isr(i2c2_er)
isr(spi1)
isr(spi2)
isr(usart1)
isr(usart2)
isr(usart3)
isr(exti15_10)
isr(rtc_alarm)
isr(usb_fs_wkup)
isr(tim6)
isr(tim7)

#define i(addr,name)	[(addr)/4] = stm_ ## name ## _isr

__attribute__ ((section(".interrupt")))
const void *stm_interrupt_vector[] = {
	[0] = &__stack__,
	[1] = start,
	i(0x08, nmi),
	i(0x0c, hardfault),
	i(0x10, memmanage),
	i(0x14, busfault),
	i(0x18, usagefault),
	i(0x2c, svc),
	i(0x30, debugmon),
	i(0x38, pendsv),
	i(0x3c, systick),
	i(0x40, wwdg),
	i(0x44, pvd),
	i(0x48, tamper_stamp),
	i(0x4c, rtc_wkup),
	i(0x50, flash),
	i(0x54, rcc),
	i(0x58, exti0),
	i(0x5c, exti1),
	i(0x60, exti2),
	i(0x64, exti3),
	i(0x68, exti4),
	i(0x6c, dma1_channel1),
	i(0x70, dma1_channel2),
	i(0x74, dma1_channel3),
	i(0x78, dma1_channel4),
	i(0x7c, dma1_channel5),
	i(0x80, dma1_channel6),
	i(0x84, dma1_channel7),
	i(0x88, adc1),
	i(0x8c, usb_hp),
	i(0x90, usb_lp),
	i(0x94, dac),
	i(0x98, comp),
	i(0x9c, exti9_5),
	i(0xa0, lcd),
	i(0xa4, tim9),
	i(0xa8, tim10),
	i(0xac, tim11),
	i(0xb0, tim2),
	i(0xb4, tim3),
	i(0xb8, tim4),
	i(0xbc, i2c1_ev),
	i(0xc0, i2c1_er),
	i(0xc4, i2c2_ev),
	i(0xc8, i2c2_er),
	i(0xcc, spi1),
	i(0xd0, spi2),
	i(0xd4, usart1),
	i(0xd8, usart2),
	i(0xdc, usart3),
	i(0xe0, exti15_10),
	i(0xe4, rtc_alarm),
	i(0xe8, usb_fs_wkup),
	i(0xec, tim6),
	i(0xf0, tim7),
};

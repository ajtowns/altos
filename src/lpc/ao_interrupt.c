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
#include <string.h>
#include <ao_boot.h>

#ifndef IS_FLASH_LOADER
#error Should define IS_FLASH_LOADER
#define IS_FLASH_LOADER	0
#endif

#if !IS_FLASH_LOADER
#define RELOCATE_INTERRUPT	1
#endif

extern void main(void);
extern char __stack__;
extern char __text_start__, __text_end__;
extern char __data_start__, __data_end__;
extern char __bss_start__, __bss_end__;
#if RELOCATE_INTERRUPT
extern char __interrupt_rom__, __interrupt_start__, __interrupt_end__;
#endif

/* Interrupt functions */

void lpc_halt_isr(void)
{
	ao_panic(AO_PANIC_CRASH);
}

void lpc_ignore_isr(void)
{
}

void start(void) {
#ifdef AO_BOOT_CHAIN
	if (ao_boot_check_chain()) {
#ifdef AO_BOOT_PIN
		ao_boot_check_pin();
#endif
	}
#endif
#if RELOCATE_INTERRUPT
	memcpy(&__interrupt_start__, &__interrupt_rom__, &__interrupt_end__ - &__interrupt_start__);
	lpc_scb.sysmemremap = LPC_SCB_SYSMEMREMAP_MAP_RAM << LPC_SCB_SYSMEMREMAP_MAP;
#endif
	memcpy(&__data_start__, &__text_end__, &__data_end__ - &__data_start__);
	memset(&__bss_start__, '\0', &__bss_end__ - &__bss_start__);
	main();
}

#define STRINGIFY(x) #x

#define isr(name) \
	void __attribute__ ((weak)) lpc_ ## name ## _isr(void); \
	_Pragma(STRINGIFY(weak lpc_ ## name ## _isr = lpc_ignore_isr))

#define isr_halt(name) \
	void __attribute__ ((weak)) lpc_ ## name ## _isr(void); \
	_Pragma(STRINGIFY(weak lpc_ ## name ## _isr = lpc_halt_isr))

isr(nmi)
isr_halt(hardfault)
isr_halt(memmanage)
isr_halt(busfault)
isr_halt(usagefault)
isr(svc)
isr(debugmon)
isr(pendsv)
isr(systick)

isr(pin_int0)	/* IRQ0 */
isr(pin_int1)
isr(pin_int2)
isr(pin_int3)
isr(pin_int4)	/* IRQ4 */
isr(pin_int5)
isr(pin_int6)
isr(pin_int7)

isr(gint0)	/* IRQ8 */
isr(gint1)
isr(ssp1)
isr(i2c)

isr(ct16b0)	/* IRQ16 */
isr(ct16b1)
isr(ct32b0)
isr(ct32b1)
isr(ssp0)	/* IRQ20 */
isr(usart)
isr(usb_irq)
isr(usb_fiq)

isr(adc)	/* IRQ24 */
isr(wwdt)
isr(bod)
isr(flash)

isr(usb_wakeup)

#define i(addr,name)	[(addr)/4] = lpc_ ## name ## _isr
#define c(addr,value)	[(addr)/4] = (value)

__attribute__ ((section(".interrupt")))
const void *lpc_interrupt_vector[] = {
	[0] = &__stack__,
	[1] = start,
	i(0x08, nmi),
	i(0x0c, hardfault),
	c(0x10, 0),
	c(0x14, 0),
	c(0x18, 0),
	c(0x1c, 0),
	c(0x20, 0),
	c(0x24, 0),
	c(0x28, 0),
	i(0x2c, svc),
	i(0x30, hardfault),
	i(0x34, hardfault),
	i(0x38, pendsv),
	i(0x3c, systick),

	i(0x40, pin_int0),	/* IRQ0 */
	i(0x44, pin_int1),
	i(0x48, pin_int2),
	i(0x4c, pin_int3),
	i(0x50, pin_int4),	/* IRQ4 */
	i(0x54, pin_int5),
	i(0x58, pin_int6),
	i(0x5c, pin_int7),

	i(0x60, gint0),		/* IRQ8 */
	i(0x64, gint1),
	i(0x68, hardfault),
	i(0x6c, hardfault),
	i(0x70, hardfault),	/* IRQ12 */
	i(0x74, hardfault),
	i(0x78, ssp1),
	i(0x7c, i2c),

	i(0x80, ct16b0),	/* IRQ16 */
	i(0x84, ct16b1),
	i(0x88, ct32b0),
	i(0x8c, ct32b1),
	i(0x90, ssp0),		/* IRQ20 */
	i(0x94, usart),
	i(0x98, usb_irq),
	i(0x9c, usb_fiq),

	i(0xa0, adc),		/* IRQ24 */
	i(0xa4, wwdt),
	i(0xa8, bod),
	i(0xac, flash),

	i(0xb0, hardfault),	/* IRQ28 */
	i(0xb4, hardfault),
	i(0xb8, usb_wakeup),
	i(0xbc, hardfault),
};

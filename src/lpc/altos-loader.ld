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

MEMORY {
	rom : ORIGIN = 0x00000000, LENGTH = 4K
	ram : ORIGIN = 0x10000000, LENGTH = 4k - 128 - 32
	usb (!x) : ORIGIN = 0x20004000 + 2K - 256, LENGTH = 256
	stack (!w) : ORIGIN = 0x10000000 + 4K - 128 - 32, LENGTH = 128
}

INCLUDE registers.ld

EXTERN (lpc_interrupt_vector)

SECTIONS {
	/*
	 * Rom contents
	 */

	.interrupt : {
		__text_start__ = .;
		*(.interrupt)	/* Interrupt vectors */

	} > rom

	.text ORIGIN(rom) + 0x100 : {
		ao_romconfig.o(.romconfig*)
		ao_product.o(.romconfig*)

		*(.text*)	/* Executable code */
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
		*(.rodata*)	/* Constants */
		__text_end__ = .;
	} > rom

	/* Boot data which must live at the start of ram so that
	 * the application and bootloader share the same addresses.
	 * This must be all uninitialized data
	 */
	.boot ORIGIN(ram) + SIZEOF(.interrupt) (NOLOAD) : {
		__boot_start__ = .;
		*(.boot*)
		__boot_end__ = .;
	} >ram

	/* Data -- relocated to RAM, but written to ROM
	 */
	.data : {
		__data_start__ = .;
		*(.data*)	/* initialized data */
		__data_end__ = .;
	} >ram AT>rom


	.bss : {
		__bss_start__ = .;
		*(.bss*)
		*(COMMON*)
		__bss_end__ = .;
	} >ram

	PROVIDE(__stack__ = ORIGIN(ram) + LENGTH(ram));
	PROVIDE(end = .);
}

ENTRY(start);

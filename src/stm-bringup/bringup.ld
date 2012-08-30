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
	rom (rx) : ORIGIN = 0x08000000, LENGTH = 128K
	ram (!w) : ORIGIN = 0x20000000, LENGTH = 16K
}

INCLUDE registers.ld

EXTERN (stm_interrupt_vector)

SECTIONS {
	/*
	 * Rom contents
	 */

	.text ORIGIN(rom) : {
		__text_start__ = .;
		*(.interrupt)	/* Interrupt vectors */

		*(.text*)	/* Executable code */
		*(.rodata*)	/* Constants */
	} > rom

	.ARM.exidx : {
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
		__text_end__ = .;
	} > rom

	. = ORIGIN(ram);
	__data_start__ = .;

	/* Data -- relocated to RAM, but written to ROM
	 */
	.data : AT (ADDR(.ARM.exidx) + SIZEOF (.ARM.exidx)) {
		*(.data)	/* initialized data */
		__data_end__ = .;
		__bss_start__ = .;
	} >ram

	.bss : {
		*(.bss)
		*(COMMON)
		__bss_end__ = .;
	} >ram

	PROVIDE(__stack__ = ORIGIN(ram) + LENGTH(ram));
	PROVIDE(end = .);

}

ENTRY(start);



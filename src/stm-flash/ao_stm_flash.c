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
#include <ao_boot.h>
#include <ao_flash_stm.h>

void
ao_panic(uint8_t reason)
{
	for (;;);
}

void
ao_put_string(__code char *s)
{
	char	c;
	while ((c = *s++))
		putchar(c);
}

void
ao_application(void)
{
	ao_boot_reboot(AO_BOOT_APPLICATION_BASE);
}

static uint32_t
ao_get_hex32(void)
{
	int8_t	n;
	uint32_t v = 0;

	for (;;) {
		n = getchar();
		if (n != ' ')
			break;
	}
	for(;;) {
		if ('0' <= n && n <= '9')
			n = n - '0';
		else if ('a' <= n && n <= 'f')
			n = n - ('a' - 10);
		else if ('A' <= n && n <= 'F')
			n = n - ('A' - 10);
		else
			break;
		v = (v << 4) | n;
		n = getchar();
	}
	return v;
}

void
ao_block_erase(void)
{
	uint32_t	addr = ao_get_hex32();
	uint32_t	*p = (uint32_t *) addr;

	ao_flash_erase_page(p);
}

void
ao_block_write(void)
{
	uint32_t	addr = ao_get_hex32();
	uint32_t	*p = (uint32_t *) addr;
	union {
		uint8_t		data8[256];
		uint32_t	data32[64];
	} u;
	uint16_t	i;

	if (addr < 0x08002000 || 0x08200000 <= addr) {
		ao_put_string("Invalid address\n");
		return;
	}
	for (i = 0; i < 256; i++)
		u.data8[i] = getchar();
	ao_flash_page(p, u.data32);
}

void
ao_block_read(void)
{
	uint32_t	addr = ao_get_hex32();
	uint8_t		*p = (uint8_t *) addr;
	uint16_t	i;
	uint8_t		c;

	for (i = 0; i < 256; i++) {
		c = *p++;
		putchar(c);
	}
}

static void
ao_show_version(void)
{
	puts("altos-loader");
	ao_put_string("manufacturer     "); puts(ao_manufacturer);
	ao_put_string("product          "); puts(ao_product);
	ao_put_string("software-version "); puts(ao_version);
}

static void
ao_flash_task(void) {
	for (;;) {
		switch (getchar()) {
		case 'v': ao_show_version(); break;
		case 'a': ao_application(); break;
		case 'X': ao_block_erase(); break;
		case 'W': ao_block_write(); break;
		case 'R': ao_block_read(); break;
		}
	}
}


int
main(void)
{
	ao_clock_init();

//	ao_timer_init();
//	ao_dma_init();
//	ao_exti_init();
	ao_usb_init();

	ao_flash_task();
	return 0;
}

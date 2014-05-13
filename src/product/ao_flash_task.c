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

#include "ao.h"
#include <ao_exti.h>
#include <ao_boot.h>
#include <ao_flash.h>
#include <ao_flash_task.h>

void
ao_panic(uint8_t reason)
{
	(void) reason;
}

void
ao_put_string(__code char *s)
{
	char	c;
	while ((c = *s++)) {
		if (c == '\n')
			ao_usb_putchar('\r');
		ao_usb_putchar(c);
	}
}

static void
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
		n = ao_usb_getchar();
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
		n = ao_usb_getchar();
	}
	return v;
}

static void
ao_block_erase(void)
{
	uint32_t	addr = ao_get_hex32();
	void		*p = (void *) addr;

	ao_flash_erase_page(p);
}

static int
ao_block_valid_address(uint32_t addr)
{
	if ((uint32_t) AO_BOOT_APPLICATION_BASE <= addr && addr <= (uint32_t) AO_BOOT_APPLICATION_BOUND - 256)
		return 1;
	return 0;
}

static void
ao_block_write(void)
{
	uint32_t	addr = ao_get_hex32();
	void		*p = (void *) addr;
	uint8_t		data[256];
	uint16_t	i;

	for (i = 0; i < 256; i++)
		data[i] = ao_usb_getchar();
	if (!ao_block_valid_address(addr))
		return;
	ao_flash_page(p, (void *) data);
}

static void
ao_block_read(void)
{
	uint32_t	addr = ao_get_hex32();
	uint8_t		*p = (uint8_t *) addr;
	uint16_t	i;
	uint8_t		c;

	if (!ao_block_valid_address(addr)) {
		for (i = 0; i < 256; i++)
			ao_usb_putchar(0xff);
		return;
	}
	for (i = 0; i < 256; i++) {
		c = *p++;
		ao_usb_putchar(c);
	}
}

static inline void
hexchar(uint8_t c)
{
	if (c > 10)
		c += 'a' - ('9' + 1);
	ao_usb_putchar(c + '0');
}

static void
ao_put_hex(uint32_t u)
{
	int8_t i;
	for (i = 28; i >= 0; i -= 4)
		hexchar((u >> i) & 0xf);
}

static void
ao_show_version(void)
{
	ao_put_string("altos-loader");
	ao_put_string("\nmanufacturer     "); ao_put_string(ao_manufacturer);
	ao_put_string("\nproduct          "); ao_put_string(ao_product);
	ao_put_string("\nflash-range      ");
	ao_put_hex((uint32_t) AO_BOOT_APPLICATION_BASE);
	ao_usb_putchar(' ');
	ao_put_hex((uint32_t) AO_BOOT_APPLICATION_BOUND);
	ao_put_string("\nsoftware-version "); ao_put_string(ao_version);
	ao_put_string("\n");
}

void
ao_flash_task(void) {
	for (;;) {
		ao_usb_flush();
		switch (ao_usb_getchar()) {
		case 'v': ao_show_version(); break;
		case 'a': ao_application(); break;
		case 'X': ao_block_erase(); break;
		case 'W': ao_block_write(); break;
		case 'R': ao_block_read(); break;
		}
	}
}

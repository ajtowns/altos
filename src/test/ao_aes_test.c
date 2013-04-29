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

#define __pdata
#define __data
#define __xdata
#define __code
#define __reentrant

#include <string.h>
#include <stdio.h>

#define AO_AES_TEST	1

#include "../aes/ao_aes_tables.c"
#include "../aes/ao_aes.c"

static uint8_t key[16];
static uint8_t text[16];
static uint8_t cbc[16];

int
main (int argc, char **argv)
{
	int i;

	ao_aes_init();
	ao_aes_set_mode(ao_aes_mode_cbc_mac);
	ao_aes_set_key(key);
	ao_aes_zero_iv();
	ao_aes_run(text, cbc);

	printf ("CBC");
	for (i = 0; i < sizeof (cbc); i++)
		printf (" %02x", cbc[i]);
	printf ("\n");
	return 0;
}

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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>

#define AO_FAT_TEST

void
ao_mutex_get(uint8_t *mutex)
{
}

void
ao_mutex_put(uint8_t *mutex)
{
}

void
ao_panic(uint8_t panic)
{
	printf ("panic %d\n", panic);
	exit(1);
}

#define AO_PANIC_BUFIO	15

#define ao_cmd_success	0

uint8_t ao_cmd_status;
uint32_t ao_cmd_lex_u32;

void
ao_cmd_decimal()
{
}

#define ao_cmd_register(x)

struct ao_cmds {
	void		(*func)(void);
	const char	*help;
};

int fs_fd;

uint8_t
ao_sdcard_read_block(uint32_t block, uint8_t *data)
{
	lseek(fs_fd, block * 512, 0);
	return read(fs_fd, data, 512) == 512;
}

uint8_t
ao_sdcard_write_block(uint32_t block, uint8_t *data)
{
	lseek(fs_fd, block * 512, 0);
	return write(fs_fd, data, 512) == 512;
}

void
ao_sdcard_init(void)
{
	fs_fd = open("fat.fs", 2);
}

#include "ao_bufio.c"
#include "ao_fat.c"

int
main(int argc, char **argv)
{
	uint8_t	data[15];
	int	len;
	ao_fat_init();
	ao_fat_test();
	if (ao_fat_open("DATALOG TXT")) {
		printf ("DATALOG.TXT\n");
		while ((len = ao_fat_read(data, sizeof (data))) > 0) {
			write(1, data, len);
		}
		ao_fat_close();
//		ao_fat_unlink("DATALOG TXT");
	}
	if (ao_fat_open("NEWFILE TXT")) {
		printf ("NEWFILE.TXT\n");
		while ((len = ao_fat_read(data, sizeof (data))) > 0) {
			write(1, data, len);
		}
		ao_fat_close();
	}
	if (ao_fat_creat ("NEWFILE TXT")) {
		for (len = 0; len < 4095; len++)
			ao_fat_write((uint8_t *) "hello, world!\n", 14);
		ao_fat_close();
	}
	return 0;
}

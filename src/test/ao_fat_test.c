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
#include <stdarg.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <openssl/md5.h>

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
	abort();
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

uint64_t	total_reads, total_writes;

uint8_t
ao_sdcard_read_block(uint32_t block, uint8_t *data)
{
	++total_reads;
	lseek(fs_fd, block * 512, 0);
	return read(fs_fd, data, 512) == 512;
}

uint8_t
ao_sdcard_write_block(uint32_t block, uint8_t *data)
{
	++total_writes;
	lseek(fs_fd, block * 512, 0);
	return write(fs_fd, data, 512) == 512;
}

char	*fs = "fs.fat";

void
ao_sdcard_init(void)
{
	char	cmd[1024];

	snprintf(cmd, sizeof(cmd), "rm -f %s && mkfs.vfat -C %s 16384", fs, fs);
	if (system (cmd) != 0) {
		fprintf(stderr, "'%s' failed\n", cmd);
		exit(1);
	}
	fs_fd = open(fs, 2);
	if (fs_fd < 0) {
		perror (fs);
		exit(1);
	}
}

#include "ao_bufio.c"
void
check_bufio(char *where)
{
	int	b;

	for (b = 0; b < AO_NUM_BUF; b++) {
		if (ao_bufio[b].busy) {
			printf ("%s: buffer %d busy. block %d seqno %u\n",
				where, b, ao_bufio[b].block, ao_bufio[b].seqno & 0xffff);
			abort();
		}
	}
}


void
check_fat(void);

#include "ao_fat.c"

/* Get the next cluster entry in the chain */
static uint16_t
ao_fat_entry_raw_read(uint16_t cluster, uint8_t fat)
{
	uint32_t	sector;
	uint16_t	offset;
	uint8_t		*buf;
	uint16_t	ret;

//	cluster -= 2;
	sector = cluster >> (SECTOR_SHIFT - 1);
	offset = (cluster << 1) & SECTOR_MASK;
	buf = ao_fat_sector_get(fat_start + fat * sectors_per_fat + sector);
	if (!buf)
		return 0;
	ret = get_u16(buf + offset);
	ao_fat_sector_put(buf, 0);
	return ret;
}

void
dump_fat(void)
{
	int	e;

	printf ("\n **** FAT ****\n\n");
	for (e = 0; e < number_cluster; e++) {
		if ((e & 0xf) == 0x0)
			printf ("%04x: ", e);
		printf (" %04x", ao_fat_entry_raw_read(e, 0));
		if ((e & 0xf) == 0xf)
			putchar ('\n');
	}
}

void
fat_list(void)
{
	uint16_t		entry = 0;
	struct ao_fat_dirent	dirent;

	printf ("  **** Root directory ****\n");
	while (ao_fat_readdir(&entry, &dirent)) {
		printf ("%04x: %-8.8s.%-3.3s %02x %04x %d\n",
			entry,
			dirent.name,
			dirent.name + 8,
			dirent.attr,
			dirent.cluster,
			dirent.size);
	}

	printf ("  **** End of root directory ****\n");
}

void
fatal(char *msg, ...)
{
	dump_fat();
	fat_list();

	va_list	l;
	va_start(l, msg);
	vfprintf(stderr, msg, l);
	va_end(l);

	abort();
}

void
check_fat(void)
{
	int	e;
	int	f;

	for (e = 0; e < number_cluster; e++) {
		uint16_t	v = ao_fat_entry_raw_read(e, 0);
		for (f = 1; f < number_fat; f++) {
			if (ao_fat_entry_raw_read(e, f) != v)
				fatal ("fats differ at %d\n", e);
		}
	}
}

uint16_t
check_file(uint16_t dent, uint16_t first_cluster, uint8_t *used)
{
	uint16_t	clusters = 0;
	uint16_t	cluster;

	if (!first_cluster)
		return 0;
	
	for (cluster = first_cluster;
	     (cluster & 0xfff8) != 0xfff8;
	     cluster = ao_fat_entry_raw_read(cluster, 0))
	{
		if (!ao_fat_cluster_valid(cluster))
			fatal("file %d: invalid cluster %04x\n", dent, cluster);
		if (used[cluster])
			fatal("file %d: duplicate cluster %04x\n", dent, cluster);
		used[cluster] = 1;
		clusters++;
	}
	return clusters;
}

void
check_fs(void)
{
	uint16_t	r;
	uint16_t	cluster, chain;
	uint8_t		*used;

	check_fat();

	used = calloc(1, number_cluster);

	for (r = 0; r < root_entries; r++) {
		uint8_t		*dent = ao_fat_root_get(r);
		uint16_t	clusters;
		uint32_t	size;
		uint16_t	first_cluster;
		uint8_t		name[11];

		if (!dent)
			fatal("cannot map dent %d\n", r);
		memcpy(name, dent+0, 11);
		first_cluster = get_u16(dent + 0x1a);
		size = get_u32(dent + 0x1c);
		ao_fat_root_put(dent, r, 0);

		if (name[0] == AO_FAT_DENT_END) {
			break;
		}

		clusters = check_file(r, first_cluster, used);
		if (size > clusters * bytes_per_cluster)
			fatal("file %d: size %u beyond clusters %d (%u)\n",
			      r, size, clusters, clusters * bytes_per_cluster);
		if (size <= (clusters - 1) * bytes_per_cluster)
			fatal("file %d: size %u too small clusters %d (%u)\n",
			      r, size, clusters, clusters * bytes_per_cluster);
	}
	for (; r < root_entries; r++) {
		uint8_t	*dent = ao_fat_root_get(r);
		if (!dent)
			fatal("cannot map dent %d\n", r);
		if (dent[0] != AO_FAT_DENT_END)
			fatal("found non-zero dent past end %d\n", r);
		ao_fat_root_put(dent, r, 0);
	}

	for (cluster = 0; cluster < 2; cluster++) {
		chain = ao_fat_entry_raw_read(cluster, 0);

		if ((chain & 0xfff8) != 0xfff8)
			fatal("cluster %d: not marked busy\n", cluster);
	}
	for (; cluster < number_cluster; cluster++) {
		chain = ao_fat_entry_raw_read(cluster, 0);

		if (chain != 0) {
			if (used[cluster] == 0)
				fatal("cluster %d: marked busy, but not in any file\n", cluster);
		} else {
			if (used[cluster] != 0)
				fatal("cluster %d: marked free, but foudn in file\n", cluster);
		}
	}
}

#define NUM_FILES	512
#define LINES_FILE	1000

uint32_t		sizes[NUM_FILES];

unsigned char		md5[NUM_FILES][MD5_DIGEST_LENGTH];

int
main(int argc, char **argv)
{
	char	name[12];
	int	id;
	MD5_CTX	ctx;
	unsigned char	md5_check[MD5_DIGEST_LENGTH];

	if (argv[1])
		fs = argv[1];

	ao_fat_init();

	check_bufio("top");
	ao_fat_setup();

	check_fs();
	check_bufio("after setup");
	printf ("   **** Creating %d files\n", NUM_FILES);

	for (id = 0; id < NUM_FILES; id++) {
		sprintf(name, "D%07dTXT", id);
		if (ao_fat_creat(name) == AO_FAT_SUCCESS) {
			int j;
			char	line[64];
			check_bufio("file created");
			MD5_Init(&ctx);
			for (j = 0; j < 1000; j++) {
				int len;
				sprintf (line, "Hello, world %d %d\r\n", id, j);
				len = strlen(line);
				ao_fat_write((uint8_t *) line, len);
				MD5_Update(&ctx, line, len);
				sizes[id] += len;
			}
			ao_fat_close();
			MD5_Final(&md5[id][0], &ctx);
			if (id == 0) {
				printf ("MD5 write %d:", id);
				for (j = 0; j < MD5_DIGEST_LENGTH; j++)
					printf(" %02x", md5[id][j]);
				printf ("\n");
			}
			check_bufio("file written");
		}
	}

	check_bufio("all files created");
	printf ("   **** All done creating files\n");
	check_fs();

	printf ("   **** Comparing %d files\n", NUM_FILES);

	for (id = 0; id < NUM_FILES; id++) {
		char	buf[337];
		sprintf(name, "D%07dTXT", id);
		if (ao_fat_open(name, AO_FAT_OPEN_READ) == AO_FAT_SUCCESS) {
			int	len;

			MD5_Init(&ctx);
			while ((len = ao_fat_read((uint8_t *) buf, sizeof(buf))) > 0) {
				MD5_Update(&ctx, buf, len);
			}
			ao_fat_close();
			MD5_Final(md5_check, &ctx);
			if (id == 0) {
				int j;
				printf ("MD5 read %d:", id);
				for (j = 0; j < MD5_DIGEST_LENGTH; j++)
					printf(" %02x", md5_check[j]);
				printf ("\n");
			}
			if (memcmp(md5_check, &md5[id][0], sizeof (md5_check)) != 0)
				fatal ("checksum failed file %d\n", id);
			check_bufio("file shown");
		}
	}

	printf ("\n    **** Total IO: read %llu write %llu\n", total_reads, total_writes);
	return 0;
}

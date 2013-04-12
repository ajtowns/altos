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

struct fs_param {
	int	fat;
	int	blocks;
} fs_params[] = {
	{ .fat = 16, .blocks = 16384 },
	{ .fat = 32, .blocks = 16384 },
	{ .fat = 16, .blocks = 65536 },
	{ .fat = 32, .blocks = 65536 },
	{ .fat = 16, .blocks = 1048576 },
	{ .fat = 32, .blocks = 1048576 },
	{ .fat = 0, .blocks = 0 },
};

char		*fs = "fs.fat";
struct fs_param	*param;

void
ao_sdcard_init(void)
{
	char	cmd[1024];

	if (fs_fd) {
		close(fs_fd);
		fs_fd = 0;
	}
	snprintf(cmd, sizeof(cmd), "rm -f %s && mkfs.vfat -F %d -C %s %d",
		 fs, param->fat, fs, param->blocks);
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
static cluster_t
ao_fat_entry_raw_read(cluster_t cluster, uint8_t fat)
{
	sector_t		sector;
	cluster_offset_t	offset;
	uint8_t			*buf;
	cluster_t		ret;

	if (fat32)
		cluster <<= 2;
	else
		cluster <<= 1;
	sector = cluster >> SECTOR_SHIFT;
	offset = cluster & SECTOR_MASK;
	buf = _ao_fat_sector_get(fat_start + fat * sectors_per_fat + sector);
	if (!buf)
		return 0;
	if (fat32)
		ret = get_u32(buf + offset);
	else
		ret = get_u16(buf + offset);
	_ao_fat_sector_put(buf, 0);
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
		if (fat32)
			printf (" %08x", ao_fat_entry_raw_read(e, 0));
		else
			printf (" %04x", ao_fat_entry_raw_read(e, 0));
		if ((e & 0xf) == 0xf)
			putchar ('\n');
	}
	if (e & 0xf)
		putchar('\n');
}

void
fat_list(void)
{
	dirent_t		entry = 0;
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
//	dump_fat();
//	fat_list();

	va_list	l;
	va_start(l, msg);
	vfprintf(stderr, msg, l);
	va_end(l);

	abort();
}

void
check_fat(void)
{
	cluster_t	e;
	int		f;

	for (e = 0; e < number_cluster; e++) {
		cluster_t	v = ao_fat_entry_raw_read(e, 0);
		for (f = 1; f < number_fat; f++) {
			cluster_t	o = ao_fat_entry_raw_read(e, f);
			if (o != v)
				fatal ("fats differ at %08x (0 %08x %d %08x)\n", e, v, f, o);
		}
	}
}

cluster_t
check_file(dirent_t dent, cluster_t first_cluster, dirent_t *used)
{
	cluster_t	clusters = 0;
	cluster_t	cluster;

	if (!first_cluster)
		return 0;
	
	for (cluster = first_cluster;
	     fat32 ? !AO_FAT_IS_LAST_CLUSTER(cluster) : !AO_FAT_IS_LAST_CLUSTER16(cluster);
	     cluster = ao_fat_entry_raw_read(cluster, 0))
	{
		if (!_ao_fat_cluster_valid(cluster))
			fatal("file %d: invalid cluster %08x\n", dent, cluster);
		if (used[cluster])
			fatal("file %d: duplicate cluster %08x also in file %d\n", dent, cluster, used[cluster]-1);
		used[cluster] = dent;
		clusters++;
	}
	return clusters;
}

void
check_fs(void)
{
	dirent_t	r;
	cluster_t	cluster, chain;
	dirent_t	*used;
	uint8_t		*dent;

	check_fat();

	used = calloc(sizeof (dirent_t), number_cluster);

	for (r = 0; (dent = _ao_fat_root_get(r)); r++) {
		cluster_t	clusters;
		offset_t	size;
		cluster_t	first_cluster;
		char		name[11];

		if (!dent)
			fatal("cannot map dent %d\n", r);
		memcpy(name, dent+0, 11);
		first_cluster = get_u16(dent + 0x1a);
		if (fat32)
			first_cluster |= (cluster_t) get_u16(dent + 0x14) << 16;
		size = get_u32(dent + 0x1c);
		_ao_fat_root_put(dent, r, 0);

		if (name[0] == AO_FAT_DENT_END) {
			break;
		}

		clusters = check_file(r + 1, first_cluster, used);
		if (size == 0) {
			if (clusters != 0)
				fatal("file %d: zero sized, but %d clusters\n", clusters);
		} else {
			if (size > clusters * bytes_per_cluster)
				fatal("file %d: size %u beyond clusters %d (%u)\n",
				      r, size, clusters, clusters * bytes_per_cluster);
			if (size <= (clusters - 1) * bytes_per_cluster)
				fatal("file %d: size %u too small clusters %d (%u)\n",
				      r, size, clusters, clusters * bytes_per_cluster);
		}
	}
	if (!fat32) {
		for (; r < root_entries; r++) {
			uint8_t	*dent = _ao_fat_root_get(r);
			if (!dent)
				fatal("cannot map dent %d\n", r);
			if (dent[0] != AO_FAT_DENT_END)
				fatal("found non-zero dent past end %d\n", r);
			_ao_fat_root_put(dent, r, 0);
		}
	} else {
		check_file((dirent_t) -1, root_cluster, used);
	}

	for (cluster = 0; cluster < 2; cluster++) {
		chain = ao_fat_entry_raw_read(cluster, 0);

		if (fat32) {
			if ((chain & 0xffffff8) != 0xffffff8)
				fatal("cluster %d: not marked busy\n", cluster);
		} else {
			if ((chain & 0xfff8) != 0xfff8)
				fatal("cluster %d: not marked busy\n", cluster);
		}
	}
	for (; cluster < number_cluster; cluster++) {
		chain = ao_fat_entry_raw_read(cluster, 0);

		if (chain != 0) {
			if (used[cluster] == 0)
				fatal("cluster %d: marked busy, but not in any file\n", cluster);
		} else {
			if (used[cluster] != 0)
				fatal("cluster %d: marked free, but found in file %d\n", cluster, used[cluster]-1);
		}
	}
}

#define NUM_FILES	100
#define LINES_FILE	500000

uint32_t		sizes[NUM_FILES];

unsigned char		md5[NUM_FILES][MD5_DIGEST_LENGTH];

void
micro_test_fs(void)
{
	int8_t	fd;
	char	name[] = "FOO        ";
	char	buf[512];
	int	len;

	printf ("write once\n");
	if ((fd = ao_fat_creat(name)) >= 0) {
		ao_fat_write(fd, "hello world\n", 12);
		ao_fat_close(fd);
	}

	printf ("read first\n");
	if ((fd = ao_fat_open(name, AO_FAT_OPEN_READ)) >= 0) {
		len = ao_fat_read(fd, buf, sizeof(buf));
		write (1, buf, len);
		ao_fat_close(fd);
	}
	
	printf ("write again\n");
	if ((fd = ao_fat_creat(name)) >= 0) {
		ao_fat_write(fd, "hi\n", 3);
		ao_fat_close(fd);
	}

	printf ("read again\n");
	if ((fd = ao_fat_open(name, AO_FAT_OPEN_READ)) >= 0) {
		len = ao_fat_read(fd, buf, sizeof(buf));
		write (1, buf, len);
		ao_fat_close(fd);
	}

	printf ("write 3\n");
	if ((fd = ao_fat_creat(name)) >= 0) {
		int	l;
		char	c;

		for (l = 0; l < 10; l++) {
			for (c = ' '; c < '~'; c++)
				ao_fat_write(fd, &c, 1);
			c = '\n';
			ao_fat_write(fd, &c, 1);
		}
		ao_fat_close(fd);
	}

	printf ("read 3\n");
	if ((fd = ao_fat_open(name, AO_FAT_OPEN_READ)) >= 0) {
		while ((len = ao_fat_read(fd, buf, sizeof(buf))) > 0)
			write (1, buf, len);
		ao_fat_close(fd);
	}

	check_fs();
	printf ("all done\n");
}


void
short_test_fs(void)
{
	int	len;
	int8_t	fd;
	char	buf[345];

	if ((fd = ao_fat_open("HELLO   TXT",AO_FAT_OPEN_READ)) >= 0) {
		printf ("File contents for HELLO.TXT\n");
		while ((len = ao_fat_read(fd, buf, sizeof(buf))))
			write(1, buf, len);
		ao_fat_close(fd);
	}
	
	if ((fd = ao_fat_creat("NEWFILE TXT")) >= 0) {
		printf ("Create new file\n");
		for (len = 0; len < 2; len++)
			ao_fat_write(fd, "hello, world!\n", 14);
		ao_fat_seek(fd, 0, AO_FAT_SEEK_SET);
		printf ("read new file\n");
		while ((len = ao_fat_read(fd, buf, sizeof (buf))))
			write (1, buf, len);
		ao_fat_close(fd);
	}

	check_fs();
}

void
long_test_fs(void)
{
	char	name[12];
	int	id;
	MD5_CTX	ctx;
	unsigned char	md5_check[MD5_DIGEST_LENGTH];
	char buf[337];
	int	len;
	int8_t	fd;
	uint64_t	total_file_size = 0;

	total_reads = total_writes = 0;

	printf ("   **** Creating %d files\n", NUM_FILES);

	memset(sizes, '\0', sizeof (sizes));
	for (id = 0; id < NUM_FILES; id++) {
		sprintf(name, "D%07dTXT", id);
		if ((id % ((NUM_FILES+49)/50)) == 0) {
			printf ("."); fflush(stdout);
		}
		if ((fd = ao_fat_creat(name)) >= 0) {
			int j;
			char	line[64];
			check_bufio("file created");
			MD5_Init(&ctx);
			for (j = 0; j < LINES_FILE; j++) {
				int len, ret;
				sprintf (line, "Hello, world %d %d\r\n", id, j);
				len = strlen(line);
				ret = ao_fat_write(fd, line, len);
				if (ret <= 0)
					break;
				total_file_size += ret;
				MD5_Update(&ctx, line, ret);
				sizes[id] += ret;
				if (ret != len)
					printf ("write failed %d\n", ret);
			}
			ao_fat_close(fd);
			MD5_Final(&md5[id][0], &ctx);
			check_bufio("file written");
		}
	}

	printf ("\n   **** Write IO: read %llu write %llu data sectors %llu\n", total_reads, total_writes, (total_file_size + 511) / 512);

	check_bufio("all files created");
	printf ("   **** All done creating files\n");
	check_fs();

	total_reads = total_writes = 0;

	printf ("   **** Comparing %d files\n", NUM_FILES);

	for (id = 0; id < NUM_FILES; id++) {
		uint32_t size;
		sprintf(name, "D%07dTXT", id);
		size = 0;
		if ((id % ((NUM_FILES+49)/50)) == 0) {
			printf ("."); fflush(stdout);
		}
		if ((fd = ao_fat_open(name, AO_FAT_OPEN_READ)) >= 0) {
			MD5_Init(&ctx);
			while ((len = ao_fat_read(fd, buf, sizeof(buf))) > 0) {
				MD5_Update(&ctx, buf, len);
				size += len;
			}
			ao_fat_close(fd);
			MD5_Final(md5_check, &ctx);
			if (size != sizes[id])
				fatal("file %d: size differs %d written %d read\n",
				      id, sizes[id], size);
			if (memcmp(md5_check, &md5[id][0], sizeof (md5_check)) != 0)
				fatal ("file %d: checksum failed\n", id);
			check_bufio("file shown");
		}
	}
	printf ("\n  **** Read IO: read %llu write %llu\n", total_reads, total_writes);
}

char *params[] = {
	"-F 16 -C %s 16384",
	"-F 32 -C %s 16384",
	"-F 16 -C %s 65536",
	"-F 32 -C %s 65536",
	"-F 16 -C %s 1048576",
	"-F 32 -C %s 1048576",
	NULL
};

void
do_test(void (*test)(void))
{
	ao_fat_init();

	check_bufio("top");
	_ao_fat_setup();

	check_fs();
	check_bufio("after setup");
	(*test)();
	ao_fat_unmount();
}

int
main(int argc, char **argv)
{
	int	p;

	if (argv[1])
		fs = argv[1];

	for (p = 0; fs_params[p].fat; p++) {
		param = &fs_params[p];

		do_test(micro_test_fs);
		do_test(short_test_fs);
		do_test(long_test_fs);
	}
	unlink (fs);

	return 0;
}

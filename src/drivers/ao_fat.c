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

#ifndef AO_FAT_TEST
#include "ao.h"
#endif

#include "ao_fat.h"
#include "ao_bufio.h"

/* Partition information, block numbers */

static uint8_t partition_type;
static uint32_t	partition_start, partition_end;

/* File system parameters */
static uint8_t	sectors_per_cluster;
static uint32_t	bytes_per_cluster;
static uint16_t	reserved_sector_count;
static uint8_t	number_fat;
static uint16_t	root_entries;
static uint16_t sectors_per_fat;
static uint32_t	fat_start;
static uint32_t root_start;
static uint32_t data_start;

static uint32_t
get_u32(uint8_t *base)
{
	return ((uint32_t) base[0] |
		((uint32_t) base[1] << 8) |
		((uint32_t) base[2] << 16) |
		((uint32_t) base[3] << 24));
}

static void
put_u32(uint8_t *base, uint32_t value)
{
	base[0] = value;
	base[1] = value >> 8;
	base[2] = value >> 16;
	base[3] = value >> 24;
}

static uint16_t
get_u16(uint8_t *base)
{
	return ((uint16_t) base[0] | ((uint16_t) base[1] << 8));
}

static void
put_u16(uint8_t *base, uint16_t value)
{
	base[0] = value;
	base[1] = value >> 8;
}

static uint8_t
ao_fat_cluster_valid(uint16_t cluster)
{
	return (2 <= cluster && cluster < 0xfff0);
}

/* Start using a block */
static uint8_t *
ao_fat_block_get(uint32_t block)
{
	block += partition_start;
	if (block >= partition_end)
		return NULL;
	return ao_bufio_get(block);
}

/* Finish using a block, 'w' is 1 if modified */
#define ao_fat_block_put(b,w) ao_bufio_put(b,w)

/* Start using a root directory entry */
static uint8_t *
ao_fat_root_get(uint16_t e)
{
	uint32_t	byte = e * 0x20;
	uint32_t	sector = byte >> 9;
	uint16_t	offset = byte & 0x1ff;
	uint8_t		*buf;

	buf = ao_fat_block_get(root_start + sector);
	if (!buf)
		return NULL;
	return buf + offset;
}

/* Finish using a root directory entry, 'w' is 1 if modified */
static void
ao_fat_root_put(uint8_t *root, uint16_t e, uint8_t write)
{
	uint16_t	offset = ((e * 0x20) & 0x1ff);
	uint8_t		*buf = root - offset;

	ao_fat_block_put(buf, write);
}

/* Get the next cluster entry in the chain */
static uint16_t
ao_fat_entry_read(uint16_t cluster)
{
	uint32_t	sector;
	uint16_t	offset;
	uint8_t		*buf;
	uint16_t	ret;

	if (!ao_fat_cluster_valid(cluster))
		return 0xfff7;

	cluster -= 2;
	sector = cluster >> 8;
	offset = (cluster << 1) & 0x1ff;
	buf = ao_fat_block_get(fat_start + sector);
	if (!buf)
		return 0;
	ret = buf[offset] | (buf[offset+1] << 8);
	ao_fat_block_put(buf, 0);
	return ret;
}

static uint16_t
ao_fat_entry_replace(uint16_t  cluster, uint16_t new_value)
{
	uint32_t	sector;
	uint16_t	offset;
	uint8_t		*buf;
	uint16_t	ret;
	uint8_t		other_fats;

	if (!ao_fat_cluster_valid(cluster))
		return 0;

	cluster -= 2;
	sector = cluster >> 8;
	offset = (cluster << 1) & 0x1ff;
	buf = ao_fat_block_get(fat_start + sector);
	if (!buf)
		return 0;
	ret = get_u16(buf + offset);
	put_u16(buf + offset, new_value);
	ao_fat_block_put(buf, 1);
	for (other_fats = 1; other_fats < number_fat; other_fats++) {
		buf = ao_fat_block_get(fat_start + other_fats * sectors_per_fat);
		if (buf) {
			put_u16(buf + offset, new_value);
			ao_fat_block_put(buf, 1);
		}
	}
	return ret;
	
}

static void
ao_fat_clear_cluster_chain(uint16_t cluster)
{
	while (ao_fat_cluster_valid(cluster))
		cluster = ao_fat_entry_replace(cluster, 0x0000);
}

static uint16_t
ao_fat_cluster_seek(uint16_t cluster, uint16_t distance)
{
	while (distance) {
		cluster = ao_fat_entry_read(cluster);
		if (!ao_fat_cluster_valid(cluster))
			break;
		distance--;
	}
	return cluster;
}

static uint32_t
ao_fat_sector_seek(uint16_t cluster, uint32_t sector)
{
	cluster = ao_fat_cluster_seek(cluster, sector / sectors_per_cluster);
	if (!ao_fat_cluster_valid(cluster))
		return 0xffffffff;
	return data_start + (cluster-2) * sectors_per_cluster + sector % sectors_per_cluster;
}

/* Load the boot block and find the first partition */
static uint8_t
ao_fat_setup_partition(void)
{
	uint8_t *mbr;
	uint8_t	*partition;
	uint32_t partition_size;

	mbr = ao_bufio_get(0);
	if (!mbr)
		return 0;

	/* Check the signature */
	if (mbr[0x1fe] != 0x55 || mbr[0x1ff] != 0xaa) {
		printf ("Invalid MBR signature %02x %02x\n",
			mbr[0x1fe], mbr[0x1ff]);
		ao_bufio_put(mbr, 0);
		return 0;
	}

	/* Just use the first partition */
	partition = &mbr[0x1be];
	
	partition_type = partition[4];
	switch (partition_type) {
	case 4:		/* FAT16 up to 32M */
	case 6:		/* FAT16 over 32M */
		break;
	case 0x0b:	/* FAT32 up to 2047GB */
	case 0x0c:	/* FAT32 LBA */
		break;
	default:
		printf ("Invalid partition type %02x\n", partition_type);
		ao_bufio_put(mbr, 0);
		return 0;
	}

	partition_start = get_u32(partition+8);
	partition_size = get_u32(partition+12);
	if (partition_size == 0) {
		printf ("Zero-sized partition\n");
		ao_bufio_put(mbr, 0);
		return 0;
	}
	partition_end = partition_start + partition_size;
	printf ("Partition type %02x start %08x end %08x\n",
		partition_type, partition_start, partition_end);
	ao_bufio_put(mbr, 0);
	return 1;
}
	
static uint8_t
ao_fat_setup_fs(void)
{
	uint8_t	*boot = ao_fat_block_get(0);

	if (!boot)
		return 0;

	/* Check the signature */
	if (boot[0x1fe] != 0x55 || boot[0x1ff] != 0xaa) {
		printf ("Invalid BOOT signature %02x %02x\n",
			boot[0x1fe], boot[0x1ff]);
		ao_bufio_put(boot, 0);
		return 0;
	}

	/* Check the sector size */
	if (get_u16(boot + 0xb) != 0x200) {
		printf ("Invalid sector size %d\n",
			get_u16(boot + 0xb));
		ao_bufio_put(boot, 0);
		return 0;
	}

	sectors_per_cluster = boot[0xd];
	bytes_per_cluster = sectors_per_cluster << 9;
	reserved_sector_count = get_u16(boot+0xe);
	number_fat = boot[0x10];
	root_entries = get_u16(boot + 0x11);
	sectors_per_fat = get_u16(boot+0x16);

	printf ("sectors per cluster %d\n", sectors_per_cluster);
	printf ("reserved sectors %d\n", reserved_sector_count);
	printf ("number of FATs %d\n", number_fat);
	printf ("root entries %d\n", root_entries);
	printf ("sectors per fat %d\n", sectors_per_fat);

	fat_start = reserved_sector_count;;
	root_start = fat_start + number_fat * sectors_per_fat;
	data_start = root_start + ((root_entries * 0x20 + 0x1ff) >> 9);

	printf ("fat  start %d\n", fat_start);
	printf ("root start %d\n", root_start);
	printf ("data start %d\n", data_start);

	return 1;
}

static uint8_t
ao_fat_setup(void)
{
	if (!ao_fat_setup_partition())
		return 0;
	if (!ao_fat_setup_fs())
		return 0;
	return 1;
}

/*
 * Low-level directory operations
 */

/*
 * Basic file operations
 */

static struct ao_fat_dirent	ao_file_dirent;
static uint32_t 		ao_file_offset;

static uint32_t
ao_file_offset_to_sector(uint32_t offset)
{
	if (offset > ao_file_dirent.size)
		return 0xffffffff;
	return ao_fat_sector_seek(ao_file_dirent.cluster, offset >> 9);
}

uint8_t
ao_fat_open(char name[11])
{
	uint16_t		entry = 0;
	struct ao_fat_dirent	dirent;

	while (ao_fat_readdir(&entry, &dirent)) {
		if (!memcmp(name, dirent.name, 11)) {
			ao_file_dirent = dirent;
			ao_file_offset = 0;
			return 1;
		}
	}
	return 0;
}



static uint8_t
ao_fat_set_size(uint32_t size)
{
	uint16_t	clear_cluster = 0;
	uint8_t		*dent;
	uint16_t	first_cluster;

	first_cluster = ao_file_dirent.cluster;
	printf ("set size to %d\n", size);
	if (size == ao_file_dirent.size)
		return 1;
	if (size == 0) {
		printf ("erase file\n");
		clear_cluster = ao_file_dirent.cluster;
		first_cluster = 0;
	} else {
		uint16_t	new_num;
		uint16_t	old_num;

		new_num = (size + bytes_per_cluster - 1) / bytes_per_cluster;
		old_num = (ao_file_dirent.size + bytes_per_cluster - 1) / bytes_per_cluster;
		if (new_num < old_num) {
			uint16_t last_cluster;

			printf("Remove %d clusters\n", old_num - new_num);
			/* Go find the last cluster we want to preserve in the file */
			last_cluster = ao_fat_cluster_seek(ao_file_dirent.cluster, new_num - 1);

			printf ("Last cluster is now %04x\n", last_cluster);
			/* Rewrite that cluster entry with 0xffff to mark the end of the chain */
			clear_cluster = ao_fat_entry_replace(last_cluster, 0xffff);
		} else if (new_num > old_num) {
			uint16_t	need;
			uint16_t	free;
			uint16_t	last_cluster;

			if (old_num)
				last_cluster = ao_fat_cluster_seek(ao_file_dirent.cluster, old_num - 1);
			else
				last_cluster = 0;

			need = new_num - old_num;
			printf ("Need %d clusters\n", need);
			/* See if there are enough free clusters in the file system */
			for (free = 2; need > 0 && (free - 2) < sectors_per_fat * 256; free++) {
				if (!ao_fat_entry_read(free)) {
					printf ("\tCluster %04x available\n", free);
					need--;
				}
			}
			/* Still need some, tell the user that we've failed */
			if (need) {
				printf ("File system full\n");
				return 0;
			}

			need = new_num - old_num;
			/* Now go allocate those clusters */
			for (free = 2; need > 0 && (free - 2) < sectors_per_fat * 256; free++) {
				if (!ao_fat_entry_read(free)) {
					printf ("\tAllocate %04x\n", free);
					if (last_cluster)
						ao_fat_entry_replace(last_cluster, free);
					else
						first_cluster = free;
					last_cluster = free;
					need--;
				}
			}
			/* Mark the new end of the chain */
			ao_fat_entry_replace(last_cluster, 0xffff);
		}
	}

	/* Deallocate clusters off the end of the file */
	if (ao_fat_cluster_valid(clear_cluster)) {
		printf ("Clear clusters starting with %04x\n", clear_cluster);
		ao_fat_clear_cluster_chain(clear_cluster);
	}

	dent = ao_fat_root_get(ao_file_dirent.entry);
	if (!dent)
		return 0;
	put_u32(dent + 0x1c, size);
	put_u16(dent + 0x1a, first_cluster);
	ao_fat_root_put(dent, ao_file_dirent.entry, 1);
	ao_file_dirent.size = size;
	ao_file_dirent.cluster = first_cluster;
	return 1;
}

uint8_t
ao_fat_creat(char name[11])
{
	uint16_t	entry;

	if (ao_fat_open(name))
		return ao_fat_set_size(0);

	for (entry = 0; entry < root_entries; entry++) {
		uint8_t	*dent = ao_fat_root_get(entry);

		if (dent[0] == AO_FAT_DENT_EMPTY ||
		    dent[0] == AO_FAT_DENT_END) {
			memmove(dent, name, 11);
			dent[0x0b] = 0x00;
			dent[0x0c] = 0x00;
			dent[0x0d] = 0x00;
			/* XXX fix time */
			put_u16(dent + 0x0e, 0);
			/* XXX fix date */
			put_u16(dent + 0x10, 0);
			/* XXX fix date */
			put_u16(dent + 0x12, 0);
			/* XXX FAT32 high cluster bytes */
			put_u16(dent + 0x14, 0);
			/* XXX fix time */
			put_u16(dent + 0x16, 0);
			/* XXX fix date */
			put_u16(dent + 0x18, 0);
			/* cluster number */
			put_u16(dent + 0x1a, 0);
			/* size */
			put_u32(dent + 0x1c, 0);
			ao_fat_root_put(dent, entry, 1);
			return ao_fat_open(name);
		}
	}
	return 0;
}

void
ao_fat_close(void)
{
	memset(&ao_file_dirent, '\0', sizeof (struct ao_fat_dirent));
	ao_bufio_flush();
}

int
ao_fat_read(uint8_t *dest, int len)
{
	uint32_t	sector;
	uint16_t	this_time;
	uint16_t	offset;
	uint8_t		*buf;
	int		ret = 0;

	if (ao_file_offset + len > ao_file_dirent.size)
		len = ao_file_dirent.size - ao_file_offset;

	while (len) {
		offset = ao_file_offset & 0x1ff;
		if (offset + len < 512)
			this_time = len;
		else
			this_time = 512 - offset;

		sector = ao_file_offset_to_sector(ao_file_offset);
		if (sector == 0xffffffff)
			break;
		buf = ao_fat_block_get(sector);
		if (!buf)
			break;
		memcpy(dest, buf + offset, this_time);
		ao_fat_block_put(buf, 0);

		ret += this_time;
		len -= this_time;
		dest += this_time;
		ao_file_offset += this_time;
	}
	return ret;
}

int
ao_fat_write(uint8_t *src, int len)
{
	uint32_t	sector;
	uint16_t	this_time;
	uint16_t	offset;
	uint8_t		*buf;
	int		ret = 0;

	if (ao_file_offset + len > ao_file_dirent.size) {
		if (!ao_fat_set_size(ao_file_offset + len))
			return 0;
	}

	while (len) {
		offset = ao_file_offset & 0x1ff;
		if (offset + len < 512)
			this_time = len;
		else
			this_time = 512 - offset;

		sector = ao_file_offset_to_sector(ao_file_offset);
		if (sector == 0xffffffff)
			break;
		buf = ao_fat_block_get(sector);
		if (!buf)
			break;
		memcpy(buf + offset, src, this_time);
		ao_fat_block_put(buf, 1);

		ret += this_time;
		len -= this_time;
		src += this_time;
		ao_file_offset += this_time;
	}
	return 0;
}

uint32_t
ao_fat_seek(int32_t pos, uint8_t whence)
{
	switch (whence) {
	case AO_FAT_SEEK_SET:
		ao_file_offset = pos;
		break;
	case AO_FAT_SEEK_CUR:
		ao_file_offset += pos;
		break;
	case AO_FAT_SEEK_END:
		ao_file_offset = ao_file_dirent.size + pos;
		break;
	}
	if (ao_file_offset > ao_file_dirent.size)
		ao_fat_set_size(ao_file_offset);
	return ao_file_offset;
}

uint8_t
ao_fat_unlink(char name[11])
{
	uint16_t		entry = 0;
	struct ao_fat_dirent	dirent;

	while (ao_fat_readdir(&entry, &dirent)) {
		if (memcmp(name, dirent.name, 11) == 0) {
			uint8_t	*next;
			uint8_t	*ent;
			uint8_t	delete;
			ao_fat_clear_cluster_chain(dirent.cluster);
			next = ao_fat_root_get(dirent.entry + 1);
			if (next && next[0] != AO_FAT_DENT_END)
				delete = AO_FAT_DENT_EMPTY;
			else
				delete = AO_FAT_DENT_END;
			if (next)
				ao_fat_root_put(next, dirent.entry + 1, 0);
			ent = ao_fat_root_get(dirent.entry);
			if (ent) {
				memset(ent, '\0', 0x20);
				*ent = delete;
				ao_fat_root_put(ent, dirent.entry, 1);
			}
			ao_bufio_flush();
			return 1;
		}
	}
	return 0;
}

uint8_t
ao_fat_rename(char old[11], char new[11])
{
	return 0;
}

uint8_t
ao_fat_readdir(uint16_t *entry, struct ao_fat_dirent *dirent)
{
	uint8_t	*dent;

	if (*entry >= root_entries)
		return 0;
	for (;;) {
		dent = ao_fat_root_get(*entry);

		if (dent[0] == AO_FAT_DENT_END) {
			ao_fat_root_put(dent, *entry, 0);
			return 0;
		}
		if (dent[0] != AO_FAT_DENT_EMPTY &&
		    (dent[0x0b] & (AO_FAT_FILE_DIRECTORY|AO_FAT_FILE_VOLUME_LABEL)) == 0)
			break;
		ao_fat_root_put(dent, *entry, 0);
		(*entry)++;
	}
	memcpy(dirent->name, dent, 11);
	dirent->attr = dent[0xb];
	dirent->size = get_u32(dent+0x1c);
	dirent->cluster = get_u16(dent+0x1a);
	dirent->entry = *entry;
	ao_fat_root_put(dent, *entry, 0);
	(*entry)++;
	return 1;
}

static void
ao_fat_list(void)
{
	uint16_t		entry = 0;
	struct ao_fat_dirent	dirent;

	while (ao_fat_readdir(&entry, &dirent)) {
		printf ("%-8.8s.%-3.3s %02x %d\n",
			dirent.name, dirent.name + 8, dirent.attr, dirent.size);
	}
}

static void
ao_fat_test(void)
{
	ao_fat_setup();
	ao_fat_list();
}

static const struct ao_cmds ao_fat_cmds[] = {
	{ ao_fat_test,	"F\0Test FAT" },
	{ 0, NULL },
};

void
ao_fat_init(void)
{
	ao_bufio_init();
	ao_cmd_register(&ao_fat_cmds[0]);
}


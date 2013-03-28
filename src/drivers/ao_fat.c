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

/* Partition information, sector numbers */

static uint8_t partition_type;
static uint32_t	partition_start, partition_end;

#define SECTOR_SIZE	512
#define SECTOR_MASK	(SECTOR_SIZE - 1)
#define SECTOR_SHIFT	9

#define DIRENT_SIZE	32

/* File system parameters */
static uint8_t	sectors_per_cluster;
static uint32_t	bytes_per_cluster;
static uint16_t	reserved_sector_count;
static uint8_t	number_fat;
static uint16_t	root_entries;
static uint16_t sectors_per_fat;
static uint16_t	number_cluster;
static uint32_t	fat_start;
static uint32_t root_start;
static uint32_t data_start;
static uint16_t	first_free_cluster;

/*
 * Deal with LSB FAT data structures
 */
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
	return (2 <= cluster && cluster < number_cluster);
}

/* Start using a sector */
static uint8_t *
ao_fat_sector_get(uint32_t sector)
{
	sector += partition_start;
	if (sector >= partition_end)
		return NULL;
	return ao_bufio_get(sector);
}

/* Finish using a sector, 'w' is 1 if modified */
#define ao_fat_sector_put(b,w) ao_bufio_put(b,w)

/* Start using a root directory entry */
static uint8_t *
ao_fat_root_get(uint16_t e)
{
	uint32_t	byte = e * DIRENT_SIZE;
	uint32_t	sector = byte >> SECTOR_SHIFT;
	uint16_t	offset = byte & SECTOR_MASK;
	uint8_t		*buf;

	buf = ao_fat_sector_get(root_start + sector);
	if (!buf)
		return NULL;
	return buf + offset;
}

/* Finish using a root directory entry, 'w' is 1 if modified */
static void
ao_fat_root_put(uint8_t *root, uint16_t e, uint8_t write)
{
	uint16_t	offset = ((e * DIRENT_SIZE) & SECTOR_MASK);
	uint8_t		*buf = root - offset;

	ao_fat_sector_put(buf, write);
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

	sector = cluster >> (SECTOR_SHIFT - 1);
	offset = (cluster << 1) & SECTOR_MASK;
	buf = ao_fat_sector_get(fat_start + sector);
	if (!buf)
		return 0;
	ret = get_u16(buf + offset);
	ao_fat_sector_put(buf, 0);
	return ret;
}

/* Replace the referenced cluster entry in the chain with
 * 'new_value'. Return the previous value.
 */
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

	sector = cluster >> (SECTOR_SHIFT - 1);
	offset = (cluster << 1) & SECTOR_MASK;
	buf = ao_fat_sector_get(fat_start + sector);
	if (!buf)
		return 0;
	ret = get_u16(buf + offset);
	put_u16(buf + offset, new_value);
	ao_fat_sector_put(buf, 1);

	/*
	 * Keep the other FATs in sync
	 */
	for (other_fats = 1; other_fats < number_fat; other_fats++) {
		buf = ao_fat_sector_get(fat_start + other_fats * sectors_per_fat + sector);
		if (buf) {
			put_u16(buf + offset, new_value);
			ao_fat_sector_put(buf, 1);
		}
	}
	return ret;
	
}

/*
 * Walk a cluster chain and mark
 * all of them as free
 */
static void
ao_fat_free_cluster_chain(uint16_t cluster)
{
	while (ao_fat_cluster_valid(cluster)) {
		if (cluster < first_free_cluster)
			first_free_cluster = cluster;
		cluster = ao_fat_entry_replace(cluster, 0x0000);
	}
}

/*
 * ao_fat_cluster_seek
 * 
 * The basic file system operation -- map a file cluster index to a
 * partition cluster number. Done by computing the cluster number and
 * then walking that many clusters from the first cluster. Returns
 * 0xffff if we walk off the end of the file or the cluster chain
 * is damaged somehow
 */
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

/*
 * ao_fat_setup_partition
 * 
 * Load the boot block and find the first partition
 */
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

	/* Check to see if it's actually a boot block, in which
	 * case it's presumably not a paritioned device
	 */

	if (mbr[0] == 0xeb) {
		partition_start = 0;
		partition_size = get_u16(mbr + 0x13);
		if (partition_size == 0)
			partition_size = get_u32(mbr + 0x20);
	} else {
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
	uint8_t		*boot = ao_fat_sector_get(0);
	uint32_t	data_sectors;

	if (!boot)
		return 0;

	/* Check the signature */
	if (boot[0x1fe] != 0x55 || boot[0x1ff] != 0xaa) {
		printf ("Invalid BOOT signature %02x %02x\n",
			boot[0x1fe], boot[0x1ff]);
		ao_fat_sector_put(boot, 0);
		return 0;
	}

	/* Check the sector size */
	if (get_u16(boot + 0xb) != SECTOR_SIZE) {
		printf ("Invalid sector size %d\n",
			get_u16(boot + 0xb));
		ao_fat_sector_put(boot, 0);
		return 0;
	}

	sectors_per_cluster = boot[0xd];
	bytes_per_cluster = sectors_per_cluster << SECTOR_SHIFT;
	reserved_sector_count = get_u16(boot+0xe);
	number_fat = boot[0x10];
	root_entries = get_u16(boot + 0x11);
	sectors_per_fat = get_u16(boot+0x16);

	fat_start = reserved_sector_count;;
	root_start = fat_start + number_fat * sectors_per_fat;
	data_start = root_start + ((root_entries * DIRENT_SIZE + SECTOR_MASK) >> SECTOR_SHIFT);

	data_sectors = (partition_end - partition_start) - data_start;

	number_cluster = data_sectors / sectors_per_cluster;

	printf ("sectors per cluster %d\n", sectors_per_cluster);
	printf ("reserved sectors %d\n", reserved_sector_count);
	printf ("number of FATs %d\n", number_fat);
	printf ("root entries %d\n", root_entries);
	printf ("sectors per fat %d\n", sectors_per_fat);

	printf ("fat  start %d\n", fat_start);
	printf ("root start %d\n", root_start);
	printf ("data start %d\n", data_start);

	ao_fat_sector_put(boot, 0);

	return 1;
}

static uint8_t
ao_fat_setup(void)
{
	if (!ao_fat_setup_partition())
		return 0;
	check_bufio("partition setup");
	if (!ao_fat_setup_fs())
		return 0;
	check_bufio("fs setup");
	return 1;
}

/*
 * Basic file operations
 */

static struct ao_fat_dirent	ao_file_dirent;
static uint32_t 		ao_file_offset;
static uint32_t			ao_file_cluster_offset;
static uint16_t			ao_file_cluster;
static uint8_t			ao_file_opened;

static uint32_t
ao_fat_current_sector(void)
{
	uint16_t	cluster_offset;
	uint32_t	sector_offset;
	uint16_t	sector_index;
	uint16_t	cluster;

	if (ao_file_offset > ao_file_dirent.size)
		return 0xffffffff;

	sector_offset = ao_file_offset >> SECTOR_SHIFT;

	if (!ao_file_cluster) {
		cluster_offset = sector_offset / sectors_per_cluster;

		cluster = ao_fat_cluster_seek(ao_file_dirent.cluster, cluster_offset);
		if (!ao_fat_cluster_valid(cluster))
			return 0xffffffff;
		ao_file_cluster = cluster;
		ao_file_cluster_offset = cluster_offset * bytes_per_cluster;
	}

	sector_index = sector_offset % sectors_per_cluster;
	return data_start + (uint32_t) (ao_file_cluster-2) * sectors_per_cluster + sector_index;
}

static void
ao_fat_set_offset(uint32_t offset)
{
	
	if (offset == 0) {
		ao_file_cluster = ao_file_dirent.cluster;
		ao_file_cluster_offset = 0;
	}
	else if (offset < ao_file_cluster_offset ||
	    ao_file_cluster_offset + bytes_per_cluster <= offset)
	{
		ao_file_cluster = 0;
	}
	ao_file_offset = offset;
}

/*
 * ao_fat_set_size
 *
 * Set the size of the current file, truncating or extending
 * the cluster chain as needed
 */
static int8_t
ao_fat_set_size(uint32_t size)
{
	uint16_t	clear_cluster = 0;
	uint8_t		*dent;
	uint16_t	first_cluster;

	first_cluster = ao_file_dirent.cluster;
	if (size == ao_file_dirent.size)
		return AO_FAT_SUCCESS;
	if (size == 0) {
		clear_cluster = ao_file_dirent.cluster;
		first_cluster = 0;
	} else {
		uint16_t	new_num;
		uint16_t	old_num;

		new_num = (size + bytes_per_cluster - 1) / bytes_per_cluster;
		old_num = (ao_file_dirent.size + bytes_per_cluster - 1) / bytes_per_cluster;
		if (new_num < old_num) {
			uint16_t last_cluster;

			/* Go find the last cluster we want to preserve in the file */
			last_cluster = ao_fat_cluster_seek(ao_file_dirent.cluster, new_num - 1);

			/* Rewrite that cluster entry with 0xffff to mark the end of the chain */
			clear_cluster = ao_fat_entry_replace(last_cluster, 0xffff);
		} else if (new_num > old_num) {
			uint16_t	need;
			uint16_t	free;
			uint16_t	last_cluster;
			uint16_t	highest_allocated = 0;

			if (old_num)
				last_cluster = ao_fat_cluster_seek(ao_file_dirent.cluster, old_num - 1);
			else
				last_cluster = 0;

			if (first_free_cluster < 2 || number_cluster <= first_free_cluster)
				first_free_cluster = 2;

			/* See if there are enough free clusters in the file system */
			need = new_num - old_num;

#define loop_cluster	for (free = first_free_cluster; need > 0;)
#define next_cluster					\
			if (++free == number_cluster)	\
				free = 2;		\
			if (free == first_free_cluster) \
				break;			\

			loop_cluster {
				if (!ao_fat_entry_read(free))
					need--;
				next_cluster;
			}
			/* Still need some, tell the user that we've failed */
			if (need)
				return -AO_FAT_ENOSPC;

			/* Now go allocate those clusters */
			need = new_num - old_num;
			loop_cluster {
				if (!ao_fat_entry_read(free)) {
					if (free > highest_allocated)
						highest_allocated = free;
					if (last_cluster)
						ao_fat_entry_replace(last_cluster, free);
					else
						first_cluster = free;
					last_cluster = free;
					need--;
				}
				next_cluster;
			}
			first_free_cluster = highest_allocated + 1;
			if (first_free_cluster >= number_cluster)
				first_free_cluster = 2;

			/* Mark the new end of the chain */
			ao_fat_entry_replace(last_cluster, 0xffff);
		}
	}

	/* Deallocate clusters off the end of the file */
	if (ao_fat_cluster_valid(clear_cluster))
		ao_fat_free_cluster_chain(clear_cluster);

	/* Update the directory entry */
	dent = ao_fat_root_get(ao_file_dirent.entry);
	if (!dent)
		return -AO_FAT_EIO;
	put_u32(dent + 0x1c, size);
	put_u16(dent + 0x1a, first_cluster);
	ao_fat_root_put(dent, ao_file_dirent.entry, 1);
	ao_file_dirent.size = size;
	ao_file_dirent.cluster = first_cluster;
	return AO_FAT_SUCCESS;
}

/*
 * ao_fat_root_init
 *
 * Initialize a root directory entry
 */
void
ao_fat_root_init(uint8_t *dent, char name[11], uint8_t attr)
{
	memset(dent, '\0', 0x20);
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

	/* XXX fix time */
	put_u16(dent + 0x16, 0);
	/* XXX fix date */
	put_u16(dent + 0x18, 0);

	/* cluster number */
	/* Low cluster bytes */
	put_u16(dent + 0x1a, 0);
	/* FAT32 high cluster bytes */
	put_u16(dent + 0x14, 0);

	/* size */
	put_u32(dent + 0x1c, 0);
}


static void
ao_fat_dirent_init(uint8_t *dent, uint16_t entry, struct ao_fat_dirent *dirent)
{
	memcpy(dirent->name, dent + 0x00, 11);
	dirent->attr = dent[0x0b];
	dirent->size = get_u32(dent+0x1c);
	dirent->cluster = get_u16(dent+0x1a);
	dirent->entry = entry;
}

/*
 * Public API
 */

/*
 * ao_fat_open
 *
 * Open an existing file.
 */
int8_t
ao_fat_open(char name[11], uint8_t mode)
{
	uint16_t		entry = 0;
	struct ao_fat_dirent	dirent;

	if (ao_file_opened)
		return -AO_FAT_EMFILE;
	
	while (ao_fat_readdir(&entry, &dirent)) {
		if (!memcmp(name, dirent.name, 11)) {
			if (AO_FAT_IS_DIR(dirent.attr))
				return -AO_FAT_EISDIR;
			if (!AO_FAT_IS_FILE(dirent.attr))
				return -AO_FAT_EPERM;
			if (mode > AO_FAT_OPEN_READ && (dirent.attr & AO_FAT_FILE_READ_ONLY))
				return -AO_FAT_EACCESS;
			ao_file_dirent = dirent;
			ao_fat_set_offset(0);
			ao_file_opened = 1;
			return AO_FAT_SUCCESS;
		}
	}
	return -AO_FAT_ENOENT;
}

/*
 * ao_fat_creat
 *
 * Open and truncate an existing file or
 * create a new file
 */
int8_t
ao_fat_creat(char name[11])
{
	uint16_t	entry;
	int8_t		status;

	if (ao_file_opened)
		return -AO_FAT_EMFILE;

	status = ao_fat_open(name, AO_FAT_OPEN_WRITE);

	switch (status) {
	case -AO_FAT_SUCCESS:
		status = ao_fat_set_size(0);
		break;
	case -AO_FAT_ENOENT:
		for (entry = 0; entry < root_entries; entry++) {
			uint8_t	*dent = ao_fat_root_get(entry);

			if (!dent) {
				status = -AO_FAT_EIO;
				ao_fat_root_put(dent, entry, 0);
				break;
			}
				
			if (dent[0] == AO_FAT_DENT_EMPTY || dent[0] == AO_FAT_DENT_END) {
				ao_fat_root_init(dent, name, AO_FAT_FILE_REGULAR);
				ao_fat_dirent_init(dent, entry,  &ao_file_dirent);
				ao_fat_root_put(dent, entry, 1);
				ao_file_opened = 1;
				ao_fat_set_offset(0);
				status = -AO_FAT_SUCCESS;
				break;
			} else {
				ao_fat_root_put(dent, entry, 0);
			}
		}
		if (entry == root_entries)
			status = -AO_FAT_ENOSPC;
	}
	return status;
}

/*
 * ao_fat_close
 *
 * Close the currently open file
 */
int8_t
ao_fat_close(void)
{
	if (!ao_file_opened)
		return -AO_FAT_EBADF;

	memset(&ao_file_dirent, '\0', sizeof (struct ao_fat_dirent));
	ao_file_offset = 0;
	ao_file_cluster = 0;
	ao_file_opened = 0;
	ao_bufio_flush();
	return AO_FAT_SUCCESS;
}

/*
 * ao_fat_read
 *
 * Read from the file
 */
int
ao_fat_read(void *dst, int len)
{
	uint8_t		*dst_b = dst;
	uint32_t	sector;
	uint16_t	this_time;
	uint16_t	offset;
	uint8_t		*buf;
	int		ret = 0;

	if (!ao_file_opened)
		return -AO_FAT_EBADF;

	if (ao_file_offset + len > ao_file_dirent.size)
		len = ao_file_dirent.size - ao_file_offset;

	if (len < 0)
		len = 0;

	while (len) {
		offset = ao_file_offset & SECTOR_MASK;
		if (offset + len < SECTOR_SIZE)
			this_time = len;
		else
			this_time = SECTOR_SIZE - offset;

		sector = ao_fat_current_sector();
		if (sector == 0xffffffff)
			break;
		buf = ao_fat_sector_get(sector);
		if (!buf) {
			ret = -AO_FAT_EIO;
			break;
		}
		memcpy(dst_b, buf + offset, this_time);
		ao_fat_sector_put(buf, 0);

		ret += this_time;
		len -= this_time;
		dst_b += this_time;
		ao_fat_set_offset(ao_file_offset + this_time);
	}
	return ret;
}

/*
 * ao_fat_write
 *
 * Write to the file, extended as necessary
 */
int
ao_fat_write(void *src, int len)
{
	uint8_t		*src_b = src;
	uint32_t	sector;
	uint16_t	this_time;
	uint16_t	offset;
	uint8_t		*buf;
	int		ret = 0;

	if (!ao_file_opened)
		return -AO_FAT_EBADF;

	if (ao_file_offset + len > ao_file_dirent.size) {
		ret = ao_fat_set_size(ao_file_offset + len);
		if (ret < 0)
			return ret;
	}

	while (len) {
		offset = ao_file_offset & SECTOR_MASK;
		if (offset + len < SECTOR_SIZE)
			this_time = len;
		else
			this_time = SECTOR_SIZE - offset;

		sector = ao_fat_current_sector();
		if (sector == 0xffffffff)
			break;
		buf = ao_fat_sector_get(sector);
		if (!buf) {
			ret = -AO_FAT_EIO;
			break;
		}
		memcpy(buf + offset, src_b, this_time);
		ao_fat_sector_put(buf, 1);

		ret += this_time;
		len -= this_time;
		src_b += this_time;
		ao_fat_set_offset(ao_file_offset + this_time);
	}
	return ret;
}

/*
 * ao_fat_seek
 *
 * Set the position for the next I/O operation
 * Note that this doesn't actually change the size
 * of the file if the requested position is beyond
 * the current file length, that would take a future
 * write
 */
int32_t
ao_fat_seek(int32_t pos, uint8_t whence)
{
	uint32_t	new_offset = ao_file_offset;

	if (!ao_file_opened)
		return -AO_FAT_EBADF;

	switch (whence) {
	case AO_FAT_SEEK_SET:
		new_offset = pos;
		break;
	case AO_FAT_SEEK_CUR:
		new_offset += pos;
		break;
	case AO_FAT_SEEK_END:
		new_offset = ao_file_dirent.size + pos;
		break;
	}
	ao_fat_set_offset(new_offset);
	return ao_file_offset;
}

/*
 * ao_fat_unlink
 *
 * Remove a file from the directory, marking
 * all clusters as free
 */
int8_t
ao_fat_unlink(char name[11])
{
	uint16_t		entry = 0;
	struct ao_fat_dirent	dirent;

	while (ao_fat_readdir(&entry, &dirent)) {
		if (memcmp(name, dirent.name, 11) == 0) {
			uint8_t	*next;
			uint8_t	*ent;
			uint8_t	delete;

			if (AO_FAT_IS_DIR(dirent.attr))
				return -AO_FAT_EISDIR;
			if (!AO_FAT_IS_FILE(dirent.attr))
				return -AO_FAT_EPERM;

			ao_fat_free_cluster_chain(dirent.cluster);
			next = ao_fat_root_get(dirent.entry + 1);
			if (next && next[0] != AO_FAT_DENT_END)
				delete = AO_FAT_DENT_EMPTY;
			else
				delete = AO_FAT_DENT_END;
			if (next)
				ao_fat_root_put(next, dirent.entry + 1, 0);
			ent = ao_fat_root_get(dirent.entry);
			if (ent) {
				memset(ent, '\0', DIRENT_SIZE);
				*ent = delete;
				ao_fat_root_put(ent, dirent.entry, 1);
			}
			ao_bufio_flush();
			return AO_FAT_SUCCESS;
		}
	}
	return -AO_FAT_ENOENT;
}

int8_t
ao_fat_rename(char old[11], char new[11])
{
	return -AO_FAT_EIO;
}

int8_t
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
		if (dent[0] != AO_FAT_DENT_EMPTY && (dent[0xb] & 0xf) != 0xf) {
			ao_fat_dirent_init(dent, *entry, dirent);
			ao_fat_root_put(dent, *entry, 0);
			(*entry)++;
			return 1;
		}
		ao_fat_root_put(dent, *entry, 0);
		(*entry)++;
	}
}

static void
ao_fat_list(void)
{
	uint16_t		entry = 0;
	struct ao_fat_dirent	dirent;

	while (ao_fat_readdir(&entry, &dirent)) {
		printf ("%-8.8s.%-3.3s %02x %04x %d\n",
			dirent.name,
			dirent.name + 8,
			dirent.attr,
			dirent.cluster,
			dirent.size);
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

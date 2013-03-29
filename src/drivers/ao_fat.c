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

/*
 * Basic file system types
 */

typedef ao_fat_offset_t		offset_t;
typedef ao_fat_sector_t		sector_t;
typedef ao_fat_cluster_t	cluster_t;
typedef ao_fat_dirent_t		dirent_t;
typedef ao_fat_cluster_offset_t	cluster_offset_t;

/* Partition information, sector numbers */

static uint8_t	partition_type;
static sector_t	partition_start, partition_end;

#define AO_FAT_BAD_CLUSTER		0xffffff7
#define AO_FAT_LAST_CLUSTER		0xfffffff
#define AO_FAT_IS_LAST_CLUSTER(c)		(((c) & 0xffffff8) == 0xffffff8)
#define AO_FAT_IS_LAST_CLUSTER16(c)	(((c) & 0xfff8) == 0xfff8)


#define SECTOR_SIZE	512
#define SECTOR_MASK	(SECTOR_SIZE - 1)
#define SECTOR_SHIFT	9

#define DIRENT_SIZE	32

/* File system parameters */
static uint8_t		sectors_per_cluster;
static uint32_t		bytes_per_cluster;
static sector_t		reserved_sector_count;
static uint8_t		number_fat;
static dirent_t		root_entries;
static sector_t 	sectors_per_fat;
static cluster_t	number_cluster;
static sector_t		fat_start;
static sector_t 	root_start;
static sector_t 	data_start;
static cluster_t	next_free;
static uint8_t		filesystem_full;

/* FAT32 extra data */
static uint8_t		fat32;
static uint8_t		fsinfo_dirty;
static cluster_t	root_cluster;
static sector_t		fsinfo_sector;
static cluster_t	free_count;

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
ao_fat_cluster_valid(cluster_t cluster)
{
	return (2 <= cluster && cluster < number_cluster);
}

/* Start using a sector */
static uint8_t *
ao_fat_sector_get(sector_t sector)
{
	sector += partition_start;
	if (sector >= partition_end)
		return NULL;
	return ao_bufio_get(sector);
}

/* Finish using a sector, 'w' is 1 if modified */
#define ao_fat_sector_put(b,w) ao_bufio_put(b,w)

/* Get the next cluster entry in the chain */
static cluster_t
ao_fat_entry_read(cluster_t cluster)
{
	sector_t	sector;
	cluster_t	offset;
	uint8_t		*buf;
	cluster_t	ret;

	if (!ao_fat_cluster_valid(cluster))
		return 0xfffffff7;

	if (fat32)
		cluster <<= 2;
	else
		cluster <<= 1;
	sector = cluster >> (SECTOR_SHIFT);
	offset = cluster & SECTOR_MASK;
	buf = ao_fat_sector_get(fat_start + sector);
	if (!buf)
		return 0;

	if (fat32) {
		ret = get_u32(buf + offset);
		ret &= 0xfffffff;
	} else {
		ret = get_u16(buf + offset);
		if (AO_FAT_IS_LAST_CLUSTER16(ret))
			ret |= 0xfff0000;
	}
	ao_fat_sector_put(buf, 0);
	return ret;
}

/* Replace the referenced cluster entry in the chain with
 * 'new_value'. Return the previous value.
 */
static cluster_t
ao_fat_entry_replace(cluster_t  cluster, cluster_t new_value)
{
	sector_t		sector;
	cluster_offset_t	offset;
	uint8_t			*buf;
	cluster_t		ret;
	cluster_t		old_value;
	uint8_t			fat;

	if (!ao_fat_cluster_valid(cluster))
		return 0xfffffff7;

	/* Convert from cluster index to byte index */
	if (fat32)
		cluster <<= 2;
	else
		cluster <<= 1;
	sector = cluster >> SECTOR_SHIFT;
	offset = cluster & SECTOR_MASK;

	new_value &= 0xfffffff;
	for (fat = 0; fat < number_fat; fat++) {
		buf = ao_fat_sector_get(fat_start + fat * sectors_per_fat + sector);
		if (!buf)
			return 0;
		if (fat32) {
			old_value = get_u32(buf + offset);
			put_u32(buf + offset, new_value | (old_value & 0xf0000000));
			if (fat == 0) {
				ret = old_value & 0xfffffff;

				/* Track the free count if it wasn't marked
				 * invalid when we mounted the file system
				 */
				if (free_count != 0xffffffff) {
					if (new_value && !ret) {
						--free_count;
						fsinfo_dirty = 1;
					} else if (!new_value && ret) {
						++free_count;
						fsinfo_dirty = 1;
					}
				}
			}
		} else {
			if (fat == 0) {
				ret = get_u16(buf + offset);
				if (AO_FAT_IS_LAST_CLUSTER16(ret))
					ret |= 0xfff0000;
			}
			put_u16(buf + offset, new_value);
		}
		ao_fat_sector_put(buf, 1);
	}
	return ret;
	
}

/*
 * Walk a cluster chain and mark
 * all of them as free
 */
static void
ao_fat_free_cluster_chain(cluster_t cluster)
{
	while (ao_fat_cluster_valid(cluster)) {
		if (cluster < next_free) {
			next_free = cluster;
			fsinfo_dirty = 1;
		}
		cluster = ao_fat_entry_replace(cluster, 0x00000000);
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
static cluster_t
ao_fat_cluster_seek(cluster_t cluster, cluster_t distance)
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
 * ao_fat_cluster_set_size
 *
 * Set the number of clusters in the specified chain,
 * freeing extra ones or alocating new ones as needed
 *
 * Returns AO_FAT_BAD_CLUSTER on allocation failure
 */

static cluster_t
ao_fat_cluster_set_size(cluster_t first_cluster, cluster_t size)
{
	cluster_t	clear_cluster = 0;

	if (size == 0) {
		clear_cluster = first_cluster;
		first_cluster = 0;
	} else {
		cluster_t	have;
		cluster_t	last_cluster = 0;
		cluster_t	next_cluster;

		/* Walk the cluster chain to the
		 * spot where it needs to change. That
		 * will either be the end of the chain (in case it needs to grow),
		 * or after the desired number of clusters, in which case it needs to shrink
		 */
		next_cluster = first_cluster;
		for (have = 0; have < size; have++) {
			last_cluster = next_cluster;
			next_cluster = ao_fat_entry_read(last_cluster);
			if (!ao_fat_cluster_valid(next_cluster))
				break;
		}

		if (have == size) {
			/* The file is large enough, truncate as needed */
			if (ao_fat_cluster_valid(next_cluster)) {
				/* Rewrite that cluster entry with 0xffff to mark the end of the chain */
				clear_cluster = ao_fat_entry_replace(last_cluster, AO_FAT_LAST_CLUSTER);
				filesystem_full = 0;
			} else {
				/* The chain is already the right length, don't mess with it */
				;
			}
		} else {
			cluster_t	need;
			cluster_t	free;

			if (filesystem_full)
				return AO_FAT_BAD_CLUSTER;

			if (next_free < 2 || number_cluster <= next_free) {
				next_free = 2;
				fsinfo_dirty = 1;
			}

			/* See if there are enough free clusters in the file system */
			need = size - have;

#define loop_cluster	for (free = next_free; need > 0;)
#define next_cluster					\
			if (++free == number_cluster)	\
				free = 2;		\
			if (free == next_free) \
				break;			\

			loop_cluster {
				if (!ao_fat_entry_read(free))
					need--;
				next_cluster;
			}
			/* Still need some, tell the user that we've failed */
			if (need) {
				filesystem_full = 1;
				return AO_FAT_BAD_CLUSTER;
			}

			/* Now go allocate those clusters and
			 * thread them onto the chain
			 */
			need = size - have;
			loop_cluster {
				if (!ao_fat_entry_read(free)) {
					next_free = free + 1;
					if (next_free >= number_cluster)
						next_free = 2;
					fsinfo_dirty = 1;
					if (last_cluster)
						ao_fat_entry_replace(last_cluster, free);
					else
						first_cluster = free;
					last_cluster = free;
					need--;
				}
				next_cluster;
			}
#undef loop_cluster
#undef next_cluster
			/* Mark the new end of the chain */
			ao_fat_entry_replace(last_cluster, AO_FAT_LAST_CLUSTER);
		}
	}

	/* Deallocate clusters off the end of the file */
	if (ao_fat_cluster_valid(clear_cluster))
		ao_fat_free_cluster_chain(clear_cluster);
	return first_cluster;
}

/* Start using a root directory entry */
static uint8_t *
ao_fat_root_get(dirent_t e)
{
	offset_t		byte = e * DIRENT_SIZE;
	sector_t		sector = byte >> SECTOR_SHIFT;
	cluster_offset_t	offset = byte & SECTOR_MASK;
	uint8_t			*buf;

	if (fat32) {
		cluster_t	cluster_distance = sector / sectors_per_cluster;
		sector_t	sector_index = sector % sectors_per_cluster;
		cluster_t	cluster = ao_fat_cluster_seek(root_cluster, cluster_distance);

		if (ao_fat_cluster_valid(cluster))
			sector = data_start + (cluster-2) * sectors_per_cluster + sector_index;
		else
			return NULL;
	} else {
		if (e >= root_entries)
			return NULL;
		sector = root_start + sector;
	}

	buf = ao_fat_sector_get(sector);
	if (!buf)
		return NULL;
	return buf + offset;
}

/* Finish using a root directory entry, 'w' is 1 if modified */
static void
ao_fat_root_put(uint8_t *root, dirent_t e, uint8_t write)
{
	cluster_offset_t	offset = ((e * DIRENT_SIZE) & SECTOR_MASK);
	uint8_t			*buf = root - offset;

	ao_fat_sector_put(buf, write);
}

/*
 * ao_fat_root_extend
 *
 * On FAT32, make the 
 */
static int8_t
ao_fat_root_extend(dirent_t ents)
{
	offset_t	byte_size;
	cluster_t	cluster_size;
	if (!fat32)
		return 0;
	
	byte_size = ents * 0x20;
	cluster_size = byte_size / bytes_per_cluster;
	if (ao_fat_cluster_set_size(root_cluster, cluster_size) != AO_FAT_BAD_CLUSTER)
		return 1;
	return 0;
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
	fat32 = 0;
	if (sectors_per_fat == 0) {
		fat32 = 1;
		sectors_per_fat = get_u32(boot+0x24);
		root_cluster = get_u32(boot+0x2c);
		fsinfo_sector = get_u16(boot + 0x30);
	}
	ao_fat_sector_put(boot, 0);

	free_count = 0xffffffff;
	next_free = 0;
	if (fat32 && fsinfo_sector) {
		uint8_t	*fsinfo = ao_fat_sector_get(fsinfo_sector);

		if (fsinfo) {
			free_count = get_u32(fsinfo + 0x1e8);
			next_free = get_u32(fsinfo + 0x1ec);
			ao_fat_sector_put(fsinfo, 0);
		}
	}

	fat_start = reserved_sector_count;;
	root_start = fat_start + number_fat * sectors_per_fat;
	data_start = root_start + ((root_entries * DIRENT_SIZE + SECTOR_MASK) >> SECTOR_SHIFT);

	data_sectors = (partition_end - partition_start) - data_start;

	number_cluster = data_sectors / sectors_per_cluster;

	printf ("fat32: %d\n", fat32);
	printf ("sectors per cluster %d\n", sectors_per_cluster);
	printf ("reserved sectors %d\n", reserved_sector_count);
	printf ("number of FATs %d\n", number_fat);
	printf ("root entries %d\n", root_entries);
	printf ("sectors per fat %d\n", sectors_per_fat);

	printf ("fat  start %d\n", fat_start);
	printf ("root start %d\n", root_start);
	printf ("data start %d\n", data_start);

	return 1;
}

/*
 * State for the current opened file
 */
static struct ao_fat_dirent	ao_file_dirent;
static uint32_t 		ao_file_offset;
static uint32_t			ao_file_cluster_offset;
static cluster_t		ao_file_cluster;
static uint8_t			ao_file_opened;

static uint8_t
ao_fat_setup(void)
{
	ao_bufio_setup();
	
	partition_type = partition_start = partition_end = 0;
	sectors_per_cluster = bytes_per_cluster = reserved_sector_count = 0;
	number_fat = root_entries = sectors_per_fat = 0;
	number_cluster = fat_start = root_start = data_start = 0;
	next_free = filesystem_full = 0;
	fat32 = fsinfo_dirty = root_cluster = fsinfo_sector = free_count = 0;
	memset(&ao_file_dirent, '\0', sizeof (ao_file_dirent));
	ao_file_offset = ao_file_cluster_offset = ao_file_cluster = ao_file_opened = 0;
	if (!ao_fat_setup_partition())
		return 0;
	if (!ao_fat_setup_fs())
		return 0;
	return 1;
}

/*
 * Basic file operations
 */

static uint32_t
ao_fat_current_sector(void)
{
	cluster_t	cluster_offset;
	uint32_t	sector_offset;
	uint16_t	sector_index;
	cluster_t	cluster;

	if (ao_file_offset > ao_file_dirent.size)
		return 0xffffffff;

	sector_offset = ao_file_offset >> SECTOR_SHIFT;

	if (!ao_file_cluster || ao_file_offset < ao_file_cluster_offset) {
		ao_file_cluster = ao_file_dirent.cluster;
		ao_file_cluster_offset = 0;
	}

	if (ao_file_cluster_offset + bytes_per_cluster <= ao_file_offset) {
		cluster_t	cluster_distance;

		cluster_offset = sector_offset / sectors_per_cluster;

		cluster_distance = cluster_offset - ao_file_cluster_offset / bytes_per_cluster;

		cluster = ao_fat_cluster_seek(ao_file_cluster, cluster_distance);

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
	uint8_t		*dent;
	cluster_t	first_cluster;
	cluster_t	have_clusters, need_clusters;

	if (size == ao_file_dirent.size)
		return AO_FAT_SUCCESS;

	first_cluster = ao_file_dirent.cluster;
	have_clusters = (ao_file_dirent.size + bytes_per_cluster - 1) / bytes_per_cluster;
	need_clusters = (size + bytes_per_cluster - 1) / bytes_per_cluster;

	if (have_clusters != need_clusters) {
		if (ao_file_cluster && size >= ao_file_cluster_offset) {
			cluster_t	offset_clusters = (ao_file_cluster_offset + bytes_per_cluster) / bytes_per_cluster;
			cluster_t	extra_clusters = need_clusters - offset_clusters;
			cluster_t	next_cluster;

			next_cluster = ao_fat_cluster_set_size(ao_file_cluster, extra_clusters);
			if (next_cluster == AO_FAT_BAD_CLUSTER)
				return -AO_FAT_ENOSPC;
		} else {
			first_cluster = ao_fat_cluster_set_size(first_cluster, need_clusters);

			if (first_cluster == AO_FAT_BAD_CLUSTER)
				return -AO_FAT_ENOSPC;
		}
	}

	/* Update the directory entry */
	dent = ao_fat_root_get(ao_file_dirent.entry);
	if (!dent)
		return -AO_FAT_EIO;
	put_u32(dent + 0x1c, size);
	put_u16(dent + 0x1a, first_cluster);
	if (fat32)
		put_u16(dent + 0x14, first_cluster >> 16);
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
	if (fat32)
		dirent->cluster |= (cluster_t) get_u16(dent + 0x14) << 16;
	dirent->entry = entry;
}

/*
 * ao_fat_flush_fsinfo
 *
 * Write out any fsinfo changes to disk
 */

void
ao_fat_flush_fsinfo(void)
{
	uint8_t	*fsinfo;

	if (!fat32)
		return;

	if (!fsinfo_dirty)
		return;
	fsinfo_dirty = 0;
	if (!fsinfo_sector)
		return;

	fsinfo = ao_fat_sector_get(fsinfo_sector);
	if (fsinfo) {
		put_u32(fsinfo + 0x1e8, free_count);
		put_u32(fsinfo + 0x1ec, next_free);
		ao_fat_sector_put(fsinfo, 1);
	}
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
	uint8_t		*dent;

	if (ao_file_opened)
		return -AO_FAT_EMFILE;

	status = ao_fat_open(name, AO_FAT_OPEN_WRITE);

	switch (status) {
	case -AO_FAT_SUCCESS:
		status = ao_fat_set_size(0);
		break;
	case -AO_FAT_ENOENT:
		entry = 0;
		for (;;) {
			dent = ao_fat_root_get(entry);
			if (!dent) {
				
				if (ao_fat_root_extend(entry))
					continue;
				status = -AO_FAT_ENOSPC;
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
			entry++;
		}
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

	ao_fat_flush_fsinfo();
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

	for (;;) {
		dent = ao_fat_root_get(*entry);
		if (!dent)
			return 0;

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

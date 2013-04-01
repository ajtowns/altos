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

#ifndef _AO_FAT_H_
#define _AO_FAT_H_

void
ao_fat_init(void);

#define AO_FAT_FILE_REGULAR		0x00
#define AO_FAT_FILE_READ_ONLY		0x01
#define AO_FAT_FILE_HIDDEN		0x02
#define AO_FAT_FILE_SYSTEM		0x04
#define AO_FAT_FILE_VOLUME_LABEL	0x08
#define AO_FAT_FILE_DIRECTORY		0x10
#define AO_FAT_FILE_ARCHIVE		0x20

#define AO_FAT_DENT_EMPTY		0xe5
#define AO_FAT_DENT_END			0x00

#define AO_FAT_IS_FILE(attr)	(((attr) & (AO_FAT_FILE_VOLUME_LABEL|AO_FAT_FILE_DIRECTORY)) == 0)
#define AO_FAT_IS_DIR(attr)	(((attr) & (AO_FAT_FILE_DIRECTORY|AO_FAT_FILE_VOLUME_LABEL)) == AO_FAT_FILE_DIRECTORY)

/* API error codes */
#define AO_FAT_SUCCESS			0
#define AO_FAT_EPERM			1
#define AO_FAT_ENOENT			2
#define AO_FAT_EIO			4
#define AO_FAT_EBADF			9
#define AO_FAT_EACCESS			13
#define AO_FAT_EEXIST			17
#define AO_FAT_ENOTDIR			20
#define AO_FAT_EISDIR			21
#define AO_FAT_EMFILE			24
#define AO_FAT_EFBIG			27
#define AO_FAT_ENOSPC			28
#define AO_FAT_EDIREOF			29

/* ao_fat_setup return values */
#define AO_FAT_FILESYSTEM_SUCCESS			0
#define AO_FAT_FILESYSTEM_MBR_READ_FAILURE		1
#define AO_FAT_FILESYSTEM_INVALID_MBR_SIGNATURE		2
#define AO_FAT_FILESYSTEM_INVALID_PARTITION_TYPE	3
#define AO_FAT_FILESYSTEM_ZERO_SIZED_PARTITION		4

#define AO_FAT_FILESYSTEM_BOOT_READ_FAILURE		5
#define AO_FAT_FILESYSTEM_INVALID_BOOT_SIGNATURE	6
#define AO_FAT_FILESYSTEM_INVALID_SECTOR_SIZE		7

void
ao_fat_sync(void);

void
ao_fat_unmount(void);

int8_t
ao_fat_full(void);

int8_t
ao_fat_open(char name[11], uint8_t mode);

#define AO_FAT_OPEN_READ		0
#define AO_FAT_OPEN_WRITE		1
#define AO_FAT_OPEN_RW			2

int8_t
ao_fat_creat(char name[11]);

int8_t
ao_fat_close(int8_t fd);

int
ao_fat_read(int8_t fd, void *dest, int len);

int
ao_fat_write(int8_t fd, void *src, int len);

#define AO_FAT_SEEK_SET	0
#define AO_FAT_SEEK_CUR	1
#define AO_FAT_SEEK_END	2

int32_t
ao_fat_seek(int8_t fd, int32_t pos, uint8_t whence);

int8_t
ao_fat_unlink(char name[11]);

int8_t
ao_fat_rename(char old[11], char new[11]);

/*
 * Byte offset within a file. Supports files up to 2GB in size
 */
typedef int32_t		ao_fat_offset_t;

/*
 * Cluster index in partition data space
 */
typedef uint32_t	ao_fat_cluster_t;

/*
 * Sector offset within partition
 */
typedef uint32_t	ao_fat_sector_t;

/*
 * Index within the root directory
 */
typedef uint16_t	ao_fat_dirent_t;

/*
 * Offset within a cluster (or sector)
 */
typedef uint16_t	ao_fat_cluster_offset_t;

struct ao_fat_dirent {
	char			name[11];
	uint8_t			attr;
	uint32_t		size;
	ao_fat_cluster_t	cluster;
	uint16_t		entry;
};

int8_t
ao_fat_readdir(uint16_t *entry, struct ao_fat_dirent *dirent);

#endif /* _AO_FAT_H_ */

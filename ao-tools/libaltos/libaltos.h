/*
 * Copyright © 2010 Keith Packard <keithp@keithp.com>
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

#ifndef _LIBALTOS_H_
#define _LIBALTOS_H_

struct altos_device {
	//%immutable;
	char				product[256];
	int				serial;
	char				path[256];
	//%mutable;
};

int altos_init(void);

void altos_fini(void);

struct altos_list *
altos_list_start(void);

int altos_list_next(struct altos_list *list, struct altos_device *device);

void altos_list_finish(struct altos_list *list);

struct altos_file *
altos_open(struct altos_device *device);

void altos_close(struct altos_file *file);

void altos_free(struct altos_file *file);

int
altos_putchar(struct altos_file *file, char c);

int
altos_flush(struct altos_file *file);

int
altos_getchar(struct altos_file *file, int timeout);

#endif /* _LIBALTOS_H_ */

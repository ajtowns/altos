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

#include <stdlib.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
# ifndef BUILD_STATIC
#  ifdef BUILD_DLL
#   define PUBLIC __declspec(dllexport)
#  else
#   define PUBLIC __declspec(dllimport)
#  endif
# endif /* BUILD_STATIC */
#endif

#ifndef PUBLIC
# define PUBLIC
#endif

struct altos_device {
	//%immutable;
	int				vendor;
	int				product;
	int				serial;
	char				name[256];
	char				path[256];
	//%mutable;
};

struct altos_bt_device {
	//%immutable;
	char				name[256];
	char				addr[20];
	//%mutable;
};

struct altos_error {
	int				code;
	char				string[1024];
};

#define LIBALTOS_SUCCESS	0
#define LIBALTOS_ERROR		-1
#define LIBALTOS_TIMEOUT	-2

/* Returns 0 for success, < 0 on error */
PUBLIC int
altos_init(void);

PUBLIC void
altos_fini(void);

PUBLIC void
altos_get_last_error(struct altos_error *error);

PUBLIC struct altos_list *
altos_list_start(void);

PUBLIC struct altos_list *
altos_ftdi_list_start(void);

/* Returns 1 for success, zero on end of list */
PUBLIC int
altos_list_next(struct altos_list *list, struct altos_device *device);

PUBLIC void
altos_list_finish(struct altos_list *list);

PUBLIC struct altos_file *
altos_open(struct altos_device *device);

PUBLIC void
altos_close(struct altos_file *file);

PUBLIC void
altos_free(struct altos_file *file);

/* Returns < 0 for error */
PUBLIC int
altos_putchar(struct altos_file *file, char c);

/* Returns < 0 for error */
PUBLIC int
altos_flush(struct altos_file *file);

/* Returns < 0 for error or timeout. timeout of 0 == wait forever */
PUBLIC int
altos_getchar(struct altos_file *file, int timeout);

PUBLIC struct altos_bt_list *
altos_bt_list_start(int inquiry_time);

PUBLIC int
altos_bt_list_next(struct altos_bt_list *list, struct altos_bt_device *device);

PUBLIC void
altos_bt_list_finish(struct altos_bt_list *list);

PUBLIC void
altos_bt_fill_in(char *name, char *addr, struct altos_bt_device *device);

PUBLIC struct altos_file *
altos_bt_open(struct altos_bt_device *device);

#endif /* _LIBALTOS_H_ */

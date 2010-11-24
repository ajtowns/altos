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

#define USB_VENDOR_FSF			0xfffe
#define USB_VENDOR_ALTUSMETRUM		USB_VENDOR_FSF
#define USB_PRODUCT_ALTUSMETRUM		0x000a
#define USB_PRODUCT_TELEMETRUM		0x000b
#define USB_PRODUCT_TELEDONGLE		0x000c
#define USB_PRODUCT_TELETERRA		0x000d
#define USB_PRODUCT_ALTUSMETRUM_MIN	0x000a
#define USB_PRODUCT_ALTUSMETRUM_MAX	0x0013

#define USB_IS_ALTUSMETRUM(v,p)	((v) == USB_VENDOR_ALTUSMETRUM && \
		(USB_PRODUCT_ALTUSMETRUM_MIN <= (p) && \
		 (p) <= USB_PRODUCT_ALTUSMETRUM_MAX))

struct altos_device {
	//%immutable;
	int				vendor;
	int				product;
	int				serial;
	char				name[256];
	char				path[256];
	//%mutable;
};

#define LIBALTOS_SUCCESS	0
#define LIBALTOS_ERROR		-1
#define LIBALTOS_TIMEOUT	-2

/* Returns 0 for success, < 0 on error */
PUBLIC int
altos_init(void);

PUBLIC void
altos_fini(void);

PUBLIC struct altos_list *
altos_list_start(void);

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

#endif /* _LIBALTOS_H_ */

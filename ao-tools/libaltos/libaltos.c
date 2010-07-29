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

#include "libaltos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int
match_dev(char *product, int serial, struct altos_device *device)
{
	struct altos_list	*list;
	int			i;

	list = altos_list_start();
	if (!list)
		return 0;
	while ((i = altos_list_next(list, device)) != 0) {
		if (product && strncmp (product, device->product, strlen(product)) != 0)
			continue;
		if (serial && serial != device->serial)
			continue;
		break;
	}
	altos_list_finish(list);
	return i;
}

#ifdef DARWIN
/* Mac OS X don't have strndup even if _GNU_SOURCE is defined */
static char *
altos_strndup (const char *s, size_t n)
{
    size_t len = strlen (s);
    char *ret;

    if (len <= n)
       return strdup (s);
    ret = malloc(n + 1);
    strncpy(ret, s, n);
    ret[n] = '\0';
    return ret;
}

#else
#define altos_strndup strndup
#endif

int
altos_find_by_arg(char *arg, char *default_product, struct altos_device *device)
{
	char	*product;
	int	serial;
	char	*end;
	char	*colon;
	int	ret;

	if (arg)
	{
		/* check for <serial> */
		serial = strtol(arg, &end, 0);
		if (end != arg) {
			if (*end != '\0')
				return 0;
			product = NULL;
		} else {
			/* check for <product>:<serial> */
			colon = strchr(arg, ':');
			if (colon) {
				product = altos_strndup(arg, colon - arg);
				serial = strtol(colon + 1, &end, 0);
				if (*end != '\0')
					return 0;
			} else {
				product = arg;
				serial = 0;
			}
		}
	} else {
		product = NULL;
		serial = 0;
	}
	if (!product && default_product)
		ret = match_dev(default_product, serial, device);
	if (!ret)
		ret = match_dev(product, serial, device);
	if (product && product != arg)
		free(product);
	return ret;
}

#ifdef LINUX

#define _GNU_SOURCE
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *
cc_fullname (char *dir, char *file)
{
	char	*new;
	int	dlen = strlen (dir);
	int	flen = strlen (file);
	int	slen = 0;

	if (dir[dlen-1] != '/')
		slen = 1;
	new = malloc (dlen + slen + flen + 1);
	if (!new)
		return 0;
	strcpy(new, dir);
	if (slen)
		strcat (new, "/");
	strcat(new, file);
	return new;
}

static char *
cc_basename(char *file)
{
	char *b;

	b = strrchr(file, '/');
	if (!b)
		return file;
	return b + 1;
}

static char *
load_string(char *dir, char *file)
{
	char	*full = cc_fullname(dir, file);
	char	line[4096];
	char	*r;
	FILE	*f;
	int	rlen;

	f = fopen(full, "r");
	free(full);
	if (!f)
		return NULL;
	r = fgets(line, sizeof (line), f);
	fclose(f);
	if (!r)
		return NULL;
	rlen = strlen(r);
	if (r[rlen-1] == '\n')
		r[rlen-1] = '\0';
	return strdup(r);
}

static int
load_hex(char *dir, char *file)
{
	char	*line;
	char	*end;
	long	i;

	line = load_string(dir, file);
	if (!line)
		return -1;
	i = strtol(line, &end, 16);
	free(line);
	if (end == line)
		return -1;
	return i;
}

static int
load_dec(char *dir, char *file)
{
	char	*line;
	char	*end;
	long	i;

	line = load_string(dir, file);
	if (!line)
		return -1;
	i = strtol(line, &end, 10);
	free(line);
	if (end == line)
		return -1;
	return i;
}

static int
dir_filter_tty_colon(const struct dirent *d)
{
	return strncmp(d->d_name, "tty:", 4) == 0;
}

static int
dir_filter_tty(const struct dirent *d)
{
	return strncmp(d->d_name, "tty", 3) == 0;
}

struct altos_usbdev {
	char	*sys;
	char	*tty;
	char	*manufacturer;
	char	*product;
	int	serial;	/* AltOS always uses simple integer serial numbers */
	int	idProduct;
	int	idVendor;
};

static char *
usb_tty(char *sys)
{
	char *base;
	int num_configs;
	int config;
	struct dirent **namelist;
	int interface;
	int num_interfaces;
	char endpoint_base[20];
	char *endpoint_full;
	char *tty_dir;
	int ntty;
	char *tty;

	base = cc_basename(sys);
	num_configs = load_hex(sys, "bNumConfigurations");
	num_interfaces = load_hex(sys, "bNumInterfaces");
	for (config = 1; config <= num_configs; config++) {
		for (interface = 0; interface < num_interfaces; interface++) {
			sprintf(endpoint_base, "%s:%d.%d",
				base, config, interface);
			endpoint_full = cc_fullname(sys, endpoint_base);

			/* Check for tty:ttyACMx style names
			 */
			ntty = scandir(endpoint_full, &namelist,
				       dir_filter_tty_colon,
				       alphasort);
			if (ntty > 0) {
				free(endpoint_full);
				tty = cc_fullname("/dev", namelist[0]->d_name + 4);
				free(namelist);
				return tty;
			}

			/* Check for tty/ttyACMx style names
			 */
			tty_dir = cc_fullname(endpoint_full, "tty");
			free(endpoint_full);
			ntty = scandir(tty_dir, &namelist,
				       dir_filter_tty,
				       alphasort);
			free (tty_dir);
			if (ntty > 0) {
				tty = cc_fullname("/dev", namelist[0]->d_name);
				free(namelist);
				return tty;
			}
		}
	}
	return NULL;
}

static struct altos_usbdev *
usb_scan_device(char *sys)
{
	struct altos_usbdev *usbdev;

	usbdev = calloc(1, sizeof (struct altos_usbdev));
	if (!usbdev)
		return NULL;
	usbdev->sys = strdup(sys);
	usbdev->manufacturer = load_string(sys, "manufacturer");
	usbdev->product = load_string(sys, "product");
	usbdev->serial = load_dec(sys, "serial");
	usbdev->idProduct = load_hex(sys, "idProduct");
	usbdev->idVendor = load_hex(sys, "idVendor");
	usbdev->tty = usb_tty(sys);
	return usbdev;
}

static void
usbdev_free(struct altos_usbdev *usbdev)
{
	free(usbdev->sys);
	free(usbdev->manufacturer);
	free(usbdev->product);
	/* this can get used as a return value */
	if (usbdev->tty)
		free(usbdev->tty);
	free(usbdev);
}

#define USB_DEVICES	"/sys/bus/usb/devices"

static int
dir_filter_dev(const struct dirent *d)
{
	const char	*n = d->d_name;
	char	c;

	while ((c = *n++)) {
		if (isdigit(c))
			continue;
		if (c == '-')
			continue;
		if (c == '.' && n != d->d_name + 1)
			continue;
		return 0;
	}
	return 1;
}

struct altos_list {
	struct altos_usbdev	**dev;
	int			current;
	int			ndev;
};

int
altos_init(void)
{
	return 1;
}

void
altos_fini(void)
{
}

struct altos_list *
altos_list_start(void)
{
	int			e;
	struct dirent		**ents;
	char			*dir;
	struct altos_usbdev	*dev;
	struct altos_list	*devs;
	int			n;

	devs = calloc(1, sizeof (struct altos_list));
	if (!devs)
		return NULL;

	n = scandir (USB_DEVICES, &ents,
		     dir_filter_dev,
		     alphasort);
	if (!n)
		return 0;
	for (e = 0; e < n; e++) {
		dir = cc_fullname(USB_DEVICES, ents[e]->d_name);
		dev = usb_scan_device(dir);
		free(dir);
		if (dev->idVendor == 0xfffe && dev->tty) {
			if (devs->dev)
				devs->dev = realloc(devs->dev,
						    devs->ndev + 1 * sizeof (struct usbdev *));
			else
				devs->dev = malloc (sizeof (struct usbdev *));
			devs->dev[devs->ndev++] = dev;
		}
	}
	free(ents);
	devs->current = 0;
	return devs;
}

int
altos_list_next(struct altos_list *list, struct altos_device *device)
{
	struct altos_usbdev *dev;
	if (list->current >= list->ndev)
		return 0;
	dev = list->dev[list->current];
	strcpy(device->product, dev->product);
	strcpy(device->path, dev->tty);
	device->serial = dev->serial;
	list->current++;
	return 1;
}

void
altos_list_finish(struct altos_list *usbdevs)
{
	int	i;

	if (!usbdevs)
		return;
	for (i = 0; i < usbdevs->ndev; i++)
		usbdev_free(usbdevs->dev[i]);
	free(usbdevs);
}

#endif

#ifdef DARWIN

#include <IOKitLib.h>
#include <IOKit/usb/USBspec.h>
#include <sys/param.h>
#include <paths.h>
#include <CFNumber.h>
#include <IOBSD.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct altos_list {
	io_iterator_t iterator;
};

static int
get_string(io_object_t object, CFStringRef entry, char *result, int result_len)
{
	CFTypeRef entry_as_string;
	Boolean got_string;

	entry_as_string = IORegistryEntrySearchCFProperty (object,
							   kIOServicePlane,
							   entry,
							   kCFAllocatorDefault,
							   kIORegistryIterateRecursively);
	if (entry_as_string) {
		got_string = CFStringGetCString(entry_as_string,
						result, result_len,
						kCFStringEncodingASCII);
    
		CFRelease(entry_as_string);
		if (got_string)
			return 1;
	}
	return 0;
}

int
altos_init(void)
{
	return 1;
}

void
altos_fini(void)
{
}

struct altos_list *
altos_list_start(void)
{
	struct altos_list *list = calloc (sizeof (struct altos_list), 1);
	CFMutableDictionaryRef matching_dictionary = IOServiceMatching("IOUSBDevice");
	UInt32 vendor = 0xfffe, product = 0x000a;
	CFNumberRef vendor_ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &vendor);
	CFNumberRef product_ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &product);
	io_iterator_t tdIterator;
	io_object_t tdObject;
  
	CFDictionaryAddValue(matching_dictionary, CFSTR(kUSBVendorID), vendor_ref);
	CFDictionaryAddValue(matching_dictionary, CFSTR(kUSBProductID), product_ref);

	IOServiceGetMatchingServices(kIOMasterPortDefault, matching_dictionary, &list->iterator);
  
	CFRelease(vendor_ref);
	CFRelease(product_ref);
	return list;
}

int
altos_list_next(struct altos_list *list, struct altos_device *device)
{
	io_object_t object;
	char serial_string[128];

	for (;;) {
		object = IOIteratorNext(list->iterator);
		if (!object)
			return 0;
  
		if (get_string (object, CFSTR("IOCalloutDevice"), device->path, sizeof (device->path)) &&
		    get_string (object, CFSTR("USB Product Name"), device->product, sizeof (device->product)) &&
		    get_string (object, CFSTR("USB Serial Number"), serial_string, sizeof (serial_string))) {
			device->serial = atoi(serial_string);
			return 1;
		}
	}
}

void
altos_list_finish(struct altos_list *list)
{
	IOObjectRelease (list->iterator);
	free(list);
}

#endif

#ifdef POSIX_TTY

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

#define USB_BUF_SIZE	64

struct altos_file {
	int				fd;
	unsigned char			out_data[USB_BUF_SIZE];
	int				out_used;
	unsigned char			in_data[USB_BUF_SIZE];
	int				in_used;
	int				in_read;
};

struct altos_file *
altos_open(struct altos_device *device)
{
	struct altos_file	*file = calloc (sizeof (struct altos_file), 1);
	int			ret;
	struct termios		term;

	if (!file)
		return NULL;

	file->fd = open(device->path, O_RDWR | O_NOCTTY);
	if (file->fd < 0) {
		perror(device->path);
		free(file);
		return NULL;
	}
	ret = tcgetattr(file->fd, &term);
	if (ret < 0) {
		perror("tcgetattr");
		close(file->fd);
		free(file);
		return NULL;
	}
	cfmakeraw(&term);
	term.c_cc[VMIN] = 0;
	term.c_cc[VTIME] = 1;
	ret = tcsetattr(file->fd, TCSAFLUSH, &term);
	if (ret < 0) {
		perror("tcsetattr");
		close(file->fd);
		free(file);
		return NULL;
	}
	return file;
}

void
altos_close(struct altos_file *file)
{
	close(file->fd);
	file->fd = -1;
}

void
altos_free(struct altos_file *file)
{
	if (file->fd != -1)
		close(file->fd);
	free(file);
}

int
altos_putchar(struct altos_file *file, char c)
{
	int	ret;

	if (file->out_used == USB_BUF_SIZE) {
		ret = altos_flush(file);
		if (ret)
			return ret;
	}
	file->out_data[file->out_used++] = c;
	if (file->out_used == USB_BUF_SIZE)
		return altos_flush(file);
	return 0;
}

int
altos_flush(struct altos_file *file)
{
	while (file->out_used) {
		int	ret;

		if (file->fd < 0)
			return -EBADF;
		ret = write (file->fd, file->out_data, file->out_used);
		if (ret < 0)
			return -errno;
		if (ret) {
			memmove(file->out_data, file->out_data + ret,
				file->out_used - ret);
			file->out_used -= ret;
		}
	}
}

int
altos_getchar(struct altos_file *file, int timeout)
{
	while (file->in_read == file->in_used) {
		int	ret;

		altos_flush(file);
		if (file->fd < 0)
			return -EBADF;
		ret = read(file->fd, file->in_data, USB_BUF_SIZE);
		if (ret < 0)
			return -errno;
		file->in_read = 0;
		file->in_used = ret;
	}
	return file->in_data[file->in_read++];
}

#endif /* POSIX_TTY */

#ifdef USE_LIBUSB
#include <libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

libusb_context	*usb_context;

int altos_init(void)
{
	int	ret;
	ret = libusb_init(&usb_context);
	if (ret)
		return ret;
	libusb_set_debug(usb_context, 3);
	return 0;
}

void altos_fini(void)
{
	libusb_exit(usb_context);
	usb_context = NULL;
}

static libusb_device **list;
static ssize_t num, current;

int altos_list_start(void)
{
	if (list)
		altos_list_finish();
	current = 0;
	num = libusb_get_device_list(usb_context, &list);
	if (num == 0) {
		current = num = 0;
		list = NULL;
		return 0;
	}
	return 1;
}

int altos_list_next(struct altos_device *device)
{
	while (current < num) {
		struct libusb_device_descriptor descriptor;
		libusb_device *usb_device = list[current++];

		if (libusb_get_device_descriptor(usb_device, &descriptor) == 0) {
			if (descriptor.idVendor == 0xfffe)
			{
				libusb_device_handle	*handle;
				if (libusb_open(usb_device, &handle) == 0) {
					char	serial_number[256];
					libusb_get_string_descriptor_ascii(handle, descriptor.iProduct,
									   device->product,
									   sizeof(device->product));
					libusb_get_string_descriptor_ascii(handle, descriptor.iSerialNumber,
									   serial_number,
									   sizeof (serial_number));
					libusb_close(handle);
					device->serial = atoi(serial_number);
					device->device = usb_device;
					return 1;
				}
			}
		}
	}
	return 0;
}

void altos_list_finish(void)
{
	if (list) {
		libusb_free_device_list(list, 1);
		list = NULL;
	}
}

#define USB_BUF_SIZE	64

struct altos_file {
	struct libusb_device		*device;
	struct libusb_device_handle	*handle;
	int				out_ep;
	int				out_size;
	int				in_ep;
	int				in_size;
	unsigned char			out_data[USB_BUF_SIZE];
	int				out_used;
	unsigned char			in_data[USB_BUF_SIZE];
	int				in_used;
	int				in_read;
};

struct altos_file *
altos_open(struct altos_device *device)
{
	struct altos_file		*file;
	struct libusb_device_handle	*handle;
	if (libusb_open(device->device, &handle) == 0) {
		int	ret;

		ret = libusb_claim_interface(handle, 1);
#if 0
		if (ret) {
			libusb_close(handle);
			return NULL;
		}
#endif
		ret = libusb_detach_kernel_driver(handle, 1);
#if 0
		if (ret) {
			libusb_close(handle);
			return NULL;
		}
#endif

		file = calloc(sizeof (struct altos_file), 1);
		file->device = libusb_ref_device(device->device);
		file->handle = handle;
		/* XXX should get these from the endpoint descriptors */
		file->out_ep = 4 | LIBUSB_ENDPOINT_OUT;
		file->out_size = 64;
		file->in_ep = 5 | LIBUSB_ENDPOINT_IN;
		file->in_size = 64;

		return file;
	}
	return NULL;
}

void
altos_close(struct altos_file *file)
{
	libusb_close(file->handle);
	libusb_unref_device(file->device);
	file->handle = NULL;
	free(file);
}

int
altos_putchar(struct altos_file *file, char c)
{
	int	ret;

	if (file->out_used == file->out_size) {
		ret = altos_flush(file);
		if (ret)
			return ret;
	}
	file->out_data[file->out_used++] = c;
	if (file->out_used == file->out_size)
		return altos_flush(file);
	return 0;
}

int
altos_flush(struct altos_file *file)
{
	while (file->out_used) {
		int	transferred;
		int	ret;

		ret = libusb_bulk_transfer(file->handle,
					   file->out_ep,
					   file->out_data,
					   file->out_used,
					   &transferred,
					   0);
		if (ret)
			return ret;
		if (transferred) {
			memmove(file->out_data, file->out_data + transferred,
				file->out_used - transferred);
			file->out_used -= transferred;
		}
	}
}

int
altos_getchar(struct altos_file *file, int timeout)
{
	while (file->in_read == file->in_used) {
		int	ret;
		int	transferred;

		altos_flush(file);
		ret = libusb_bulk_transfer(file->handle,
					   file->in_ep,
					   file->in_data,
					   file->in_size,
					   &transferred,
					   (unsigned int) timeout);
		if (ret)
			return ret;
		file->in_read = 0;
		file->in_used = transferred;
	}
	return file->in_data[file->in_read++];
}

#endif /* USE_LIBUSB */

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

#define USE_DARWIN

#ifdef USE_DARWIN

#include <IOKitLib.h>
#include <IOKit/usb/USBspec.h>
#include <sys/param.h>
#include <paths.h>
#include <CFNumber.h>
#include <IOBSD.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <errno.h>

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


#define USB_BUF_SIZE	64

struct altos_file {
	int				fd;
	unsigned char			out_data[USB_BUF_SIZE];
	int				out_used;
	unsigned char			in_data[USB_BUF_SIZE];
	int				in_used;
	int				in_read;
};

void
altos_test(char *path)
{
	int n;
	char buf[16];
	int fd;
	struct termios term;

	fd = open(path, O_RDWR | O_NOCTTY);
	if (fd < 0) {
		perror(path);
		return;
	}
	if (ioctl(fd, TIOCEXCL, (char *) 0) < 0) {
		perror("TIOCEXCL");
		close (fd);
		return;
	}

	n = tcgetattr(fd, &term);
	if (n < 0) {
		perror("tcgetattr");
		close(fd);
		return;
	}
	cfmakeraw(&term);
	term.c_cc[VMIN] = 0;
	term.c_cc[VTIME] = 1;
	n = tcsetattr(fd, TCSAFLUSH, &term);
	if (n < 0) {
		perror("tcsetattr");
		close(fd);
		return;
	}
	write(fd, "\n?\n", 3);
	for (;;) {
		n = read(fd, buf, sizeof (buf));
		if (n < 0) {
			perror("read");
			break;
		}
		if (n == 0)
			break;
		write(1, buf, n);
	}
	close(fd);
}

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
		ret = read(file->fd, file->in_data, USB_BUF_SIZE);
		if (ret < 0)
			return -errno;
		file->in_read = 0;
		file->in_used = ret;
	}
	return file->in_data[file->in_read++];
}

#endif /* USE_DARWIN */

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

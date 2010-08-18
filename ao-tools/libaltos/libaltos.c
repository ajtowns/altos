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

#define BUILD_DLL
#include "libaltos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PUBLIC int
altos_init(void)
{
	return LIBALTOS_SUCCESS;
}

PUBLIC void
altos_fini(void)
{
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

/*
 * Scan for Altus Metrum devices by looking through /sys
 */

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
	char	*product_name;
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
	usbdev->product_name = load_string(sys, "product");
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
	free(usbdev->product_name);
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
		if (USB_IS_ALTUSMETRUM(dev->idVendor, dev->idProduct)) {
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
	strcpy(device->name, dev->product_name);
	device->vendor = dev->idVendor;
	device->product = dev->idProduct;
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
	if (file->fd != -1) {
		close(file->fd);
		file->fd = -1;
	}
}

void
altos_free(struct altos_file *file)
{
	altos_close(file);
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

#include <poll.h>

int
altos_getchar(struct altos_file *file, int timeout)
{
	while (file->in_read == file->in_used) {
		int	ret;

		altos_flush(file);
		if (file->fd < 0)
			return -EBADF;
		if (timeout) {
			struct pollfd fd;
			int ret;
			fd.fd = file->fd;
			fd.events = POLLIN;
			ret = poll(&fd, 1, timeout);
			if (ret == 0)
				return LIBALTOS_TIMEOUT;
		}
		ret = read(file->fd, file->in_data, USB_BUF_SIZE);
		if (ret < 0)
			return LIBALTOS_ERROR;
		file->in_read = 0;
		file->in_used = ret;
	}
	return file->in_data[file->in_read++];
}

#endif /* POSIX_TTY */

#ifdef WINDOWS

#include <windows.h>
#include <setupapi.h>

struct altos_list {
	HDEVINFO	dev_info;
	int		index;
};

#define USB_BUF_SIZE	64

struct altos_file {
	HANDLE				handle;
	unsigned char			out_data[USB_BUF_SIZE];
	int				out_used;
	unsigned char			in_data[USB_BUF_SIZE];
	int				in_used;
	int				in_read;
};


PUBLIC struct altos_list *
altos_list_start(void)
{
	struct altos_list	*list = calloc(1, sizeof (struct altos_list));

	if (!list)
		return NULL;
	list->dev_info = SetupDiGetClassDevs(NULL, "USB", NULL,
					     DIGCF_ALLCLASSES|DIGCF_PRESENT);
	if (list->dev_info == INVALID_HANDLE_VALUE) {
		printf("SetupDiGetClassDevs failed %d\n", GetLastError());
		free(list);
		return NULL;
	}
	list->index = 0;
	return list;
}

PUBLIC int
altos_list_next(struct altos_list *list, struct altos_device *device)
{
	SP_DEVINFO_DATA dev_info_data;
	char		port[128];
	DWORD		port_len;
	char		location[256];
	char		symbolic[256];
	DWORD		symbolic_len;
	HKEY		dev_key;
	int		vid, pid;
	int		serial;
	HRESULT 	result;
	DWORD		location_type;
	DWORD		location_len;

	dev_info_data.cbSize = sizeof (SP_DEVINFO_DATA);
	while(SetupDiEnumDeviceInfo(list->dev_info, list->index,
				    &dev_info_data))
	{
		list->index++;

		dev_key = SetupDiOpenDevRegKey(list->dev_info, &dev_info_data,
					       DICS_FLAG_GLOBAL, 0, DIREG_DEV,
					       KEY_READ);
		if (dev_key == INVALID_HANDLE_VALUE) {
			printf("cannot open device registry key\n");
			continue;
		}

		/* Fetch symbolic name for this device and parse out
		 * the vid/pid/serial info */
		symbolic_len = sizeof(symbolic);
		result = RegQueryValueEx(dev_key, "SymbolicName", NULL, NULL,
					 symbolic, &symbolic_len);
		if (result != 0) {
			printf("cannot find SymbolicName value\n");
			RegCloseKey(dev_key);
			continue;
		}
		vid = pid = serial = 0;
		sscanf(symbolic + sizeof("\\??\\USB#VID_") - 1,
		       "%04X", &vid);
		sscanf(symbolic + sizeof("\\??\\USB#VID_XXXX&PID_") - 1,
		       "%04X", &pid);
		sscanf(symbolic + sizeof("\\??\\USB#VID_XXXX&PID_XXXX#") - 1,
		       "%d", &serial);
		if (!USB_IS_ALTUSMETRUM(vid, pid)) {
			printf("Not Altus Metrum symbolic name: %s\n",
			       symbolic);
			RegCloseKey(dev_key);
			continue;
		}

		/* Fetch the com port name */
		port_len = sizeof (port);
		result = RegQueryValueEx(dev_key, "PortName", NULL, NULL,
					 port, &port_len);
		RegCloseKey(dev_key);
		if (result != 0) {
			printf("failed to get PortName\n");
			continue;
		}

		/* Fetch the 'location information' which is the device name,
		 * at least on XP */
		location_len = sizeof (location);
		if(!SetupDiGetDeviceRegistryProperty(list->dev_info,
						     &dev_info_data,
						     SPDRP_LOCATION_INFORMATION,
						     &location_type,
						     (BYTE *)location,
						     sizeof(location),
						     &location_len))
		{
			printf("Failed to get location\n");
			continue;
		}
		device->vendor = vid;
		device->product = pid;
		device->serial = serial;

		if (strcasestr(location, "tele"))
			strcpy(device->name, location);
		else
			strcpy(device->name, "");

		strcpy(device->path, port);
		printf ("product: %04x:%04x (%s)  path: %s serial %d\n",
			device->vendor, device->product, device->name,
			device->path, device->serial);
		return 1;
	}
	result = GetLastError();
	if (result != ERROR_NO_MORE_ITEMS)
		printf ("SetupDiEnumDeviceInfo failed error %d\n", result);
	return 0;
}

PUBLIC void
altos_list_finish(struct altos_list *list)
{
	SetupDiDestroyDeviceInfoList(list->dev_info);
	free(list);
}

static int
altos_fill(struct altos_file *file, int timeout)
{
	DWORD	result;
	DWORD	got;
	COMMTIMEOUTS timeouts;

	if (file->in_read < file->in_used)
		return LIBALTOS_SUCCESS;
	file->in_read = file->in_used = 0;

	if (timeout) {
		timeouts.ReadIntervalTimeout = MAXDWORD;
		timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
		timeouts.ReadTotalTimeoutConstant = timeout;
	} else {
		timeouts.ReadIntervalTimeout = 0;
		timeouts.ReadTotalTimeoutMultiplier = 0;
		timeouts.ReadTotalTimeoutConstant = 0;
	}
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;

	if (!SetCommTimeouts(file->handle, &timeouts)) {
		printf("SetCommTimeouts failed %d\n", GetLastError());
	}

	if (!ReadFile(file->handle, file->in_data, USB_BUF_SIZE, &got, NULL)) {
		result = GetLastError();
		printf ("read failed %d\n", result);
		return LIBALTOS_ERROR;
		got = 0;
	}
	if (got)
		return LIBALTOS_SUCCESS;
	return LIBALTOS_TIMEOUT;
}

PUBLIC int
altos_flush(struct altos_file *file)
{
	DWORD	put;
	char	*data = file->out_data;
	char	used = file->out_used;
	DWORD	result;

	while (used) {
		if (!WriteFile(file->handle, data, used, &put, NULL)) {
			result = GetLastError();
			printf ("write failed %d\n", result);
			return LIBALTOS_ERROR;
		}
		data += put;
		used -= put;
	}
	file->out_used = 0;
	return LIBALTOS_SUCCESS;
}

PUBLIC struct altos_file *
altos_open(struct altos_device *device)
{
	struct altos_file	*file = calloc (sizeof (struct altos_file), 1);
	char	full_name[64];

	if (!file)
		return NULL;

	strcpy(full_name, "\\\\.\\");
	strcat(full_name, device->path);
	file->handle = CreateFile(full_name, GENERIC_READ|GENERIC_WRITE,
				  0, NULL, OPEN_EXISTING,
				  FILE_ATTRIBUTE_NORMAL, NULL);
	if (file->handle == INVALID_HANDLE_VALUE) {
		free(file);
		return NULL;
	}

	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
	timeouts.ReadTotalTimeoutConstant = 100;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 10000;
	if (!SetCommTimeouts(file->handle, &timeouts)) {
		printf("SetCommTimeouts failed %d\n", GetLastError());
	}

	return file;
}

PUBLIC void
altos_close(struct altos_file *file)
{
	if (file->handle != INVALID_HANDLE_VALUE) {
		CloseHandle(file->handle);
		file->handle = INVALID_HANDLE_VALUE;
	}
}

PUBLIC void
altos_free(struct altos_file *file)
{
	altos_close(file);
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
	return LIBALTOS_SUCCESS;
}

int
altos_getchar(struct altos_file *file, int timeout)
{
	int	ret;
	while (file->in_read == file->in_used) {
		ret = altos_flush(file);
		if (ret)
			return ret;
		if (file->handle == INVALID_HANDLE_VALUE)
			return LIBALTOS_ERROR;
		ret = altos_fill(file, timeout);
		if (ret)
			return ret;
	}
	return file->in_data[file->in_read++];
}

#endif

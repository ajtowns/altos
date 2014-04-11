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

#define BLUETOOTH_PRODUCT_TELEBT	"TeleBT"

#define USE_POLL

PUBLIC int
altos_init(void)
{
	return LIBALTOS_SUCCESS;
}

PUBLIC void
altos_fini(void)
{
}

static struct altos_error last_error;

static void
altos_set_last_error(int code, char *string)
{
	last_error.code = code;
	strncpy(last_error.string, string, sizeof (last_error.string) -1);
	last_error.string[sizeof(last_error.string)-1] = '\0';
}

PUBLIC void
altos_get_last_error(struct altos_error *error)
{
	*error = last_error;
}

#ifdef DARWIN

#include <unistd.h>

#undef USE_POLL

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

#ifdef POSIX_TTY

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

#define USB_BUF_SIZE	64

struct altos_file {
	int				fd;
#ifdef USE_POLL
	int				pipe[2];
#else
	int				out_fd;
#endif
	unsigned char			out_data[USB_BUF_SIZE];
	int				out_used;
	unsigned char			in_data[USB_BUF_SIZE];
	int				in_used;
	int				in_read;
};

static void
altos_set_last_posix_error(void)
{
	altos_set_last_error(errno, strerror(errno));
}

PUBLIC struct altos_file *
altos_open(struct altos_device *device)
{
	struct altos_file	*file = calloc (sizeof (struct altos_file), 1);
	int			ret;
	struct termios		term;

	if (!file) {
		altos_set_last_posix_error();
		return NULL;
	}

//	altos_set_last_error(12, "yeah yeah, failed again");
//	free(file);
//	return NULL;

	file->fd = open(device->path, O_RDWR | O_NOCTTY);
	if (file->fd < 0) {
		altos_set_last_posix_error();
		free(file);
		return NULL;
	}
#ifdef USE_POLL
	pipe(file->pipe);
#else
	file->out_fd = open(device->path, O_RDWR | O_NOCTTY);
	if (file->out_fd < 0) {
		altos_set_last_posix_error();
		close(file->fd);
		free(file);
		return NULL;
	}
#endif
	ret = tcgetattr(file->fd, &term);
	if (ret < 0) {
		altos_set_last_posix_error();
		close(file->fd);
#ifndef USE_POLL
		close(file->out_fd);
#endif
		free(file);
		return NULL;
	}
	cfmakeraw(&term);
	cfsetospeed(&term, B9600);
	cfsetispeed(&term, B9600);
#ifdef USE_POLL
	term.c_cc[VMIN] = 1;
	term.c_cc[VTIME] = 0;
#else
	term.c_cc[VMIN] = 0;
	term.c_cc[VTIME] = 1;
#endif
	ret = tcsetattr(file->fd, TCSAFLUSH, &term);
	if (ret < 0) {
		altos_set_last_posix_error();
		close(file->fd);
#ifndef USE_POLL
		close(file->out_fd);
#endif
		free(file);
		return NULL;
	}
	return file;
}

PUBLIC void
altos_close(struct altos_file *file)
{
	if (file->fd != -1) {
		int	fd = file->fd;
		file->fd = -1;
#ifdef USE_POLL
		write(file->pipe[1], "\r", 1);
#else
		close(file->out_fd);
		file->out_fd = -1;
#endif
		close(fd);
	}
}

PUBLIC void
altos_free(struct altos_file *file)
{
	altos_close(file);
	free(file);
}

PUBLIC int
altos_flush(struct altos_file *file)
{
	if (file->out_used && 0) {
		printf ("flush \"");
		fwrite(file->out_data, 1, file->out_used, stdout);
		printf ("\"\n");
	}
	while (file->out_used) {
		int	ret;

		if (file->fd < 0)
			return -EBADF;
#ifdef USE_POLL
		ret = write (file->fd, file->out_data, file->out_used);
#else
		ret = write (file->out_fd, file->out_data, file->out_used);
#endif
		if (ret < 0) {
			altos_set_last_posix_error();
			return -last_error.code;
		}
		if (ret) {
			memmove(file->out_data, file->out_data + ret,
				file->out_used - ret);
			file->out_used -= ret;
		}
	}
	return 0;
}

PUBLIC int
altos_putchar(struct altos_file *file, char c)
{
	int	ret;

	if (file->out_used == USB_BUF_SIZE) {
		ret = altos_flush(file);
		if (ret) {
			return ret;
		}
	}
	file->out_data[file->out_used++] = c;
	ret = 0;
	if (file->out_used == USB_BUF_SIZE)
		ret = altos_flush(file);
	return ret;
}

#ifdef USE_POLL
#include <poll.h>
#endif

static int
altos_fill(struct altos_file *file, int timeout)
{
	int		ret;
#ifdef USE_POLL
	struct pollfd	fd[2];
#endif

	if (timeout == 0)
		timeout = -1;
	while (file->in_read == file->in_used) {
		if (file->fd < 0)
			return LIBALTOS_ERROR;
#ifdef USE_POLL
		fd[0].fd = file->fd;
		fd[0].events = POLLIN|POLLERR|POLLHUP|POLLNVAL;
		fd[1].fd = file->pipe[0];
		fd[1].events = POLLIN;
		ret = poll(fd, 2, timeout);
		if (ret < 0) {
			altos_set_last_posix_error();
			return LIBALTOS_ERROR;
		}
		if (ret == 0)
			return LIBALTOS_TIMEOUT;

		if (fd[0].revents & (POLLHUP|POLLERR|POLLNVAL))
			return LIBALTOS_ERROR;
		if (fd[0].revents & POLLIN)
#endif
		{
			ret = read(file->fd, file->in_data, USB_BUF_SIZE);
			if (ret < 0) {
				altos_set_last_posix_error();
				return LIBALTOS_ERROR;
			}
			file->in_read = 0;
			file->in_used = ret;
#ifndef USE_POLL
			if (ret == 0 && timeout > 0)
				return LIBALTOS_TIMEOUT;
#endif
		}
	}
	if (file->in_used && 0) {
		printf ("fill \"");
		fwrite(file->in_data, 1, file->in_used, stdout);
		printf ("\"\n");
	}
	return 0;
}

PUBLIC int
altos_getchar(struct altos_file *file, int timeout)
{
	int	ret;
	while (file->in_read == file->in_used) {
		if (file->fd < 0)
			return LIBALTOS_ERROR;
		ret = altos_fill(file, timeout);
		if (ret)
			return ret;
	}
	return file->in_data[file->in_read++];
}

#endif /* POSIX_TTY */

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
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>

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
			ntty = scandir(tty_dir, &namelist,
				       dir_filter_tty,
				       alphasort);
			free (tty_dir);
			if (ntty > 0) {
				tty = cc_fullname("/dev", namelist[0]->d_name);
				free(endpoint_full);
				free(namelist);
				return tty;
			}

			/* Check for ttyACMx style names
			 */
			ntty = scandir(endpoint_full, &namelist,
				       dir_filter_tty,
				       alphasort);
			free(endpoint_full);
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
	char *tty;

	tty = usb_tty(sys);
	if (!tty)
		return NULL;
	usbdev = calloc(1, sizeof (struct altos_usbdev));
	if (!usbdev)
		return NULL;
	usbdev->sys = strdup(sys);
	usbdev->manufacturer = load_string(sys, "manufacturer");
	usbdev->product_name = load_string(sys, "product");
	usbdev->serial = load_dec(sys, "serial");
	usbdev->idProduct = load_hex(sys, "idProduct");
	usbdev->idVendor = load_hex(sys, "idVendor");
	usbdev->tty = tty;
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
		if (!dev)
			continue;
		free(dir);
		if (devs->dev)
			devs->dev = realloc(devs->dev,
					    (devs->ndev + 1) * sizeof (struct usbdev *));
		else
			devs->dev = malloc (sizeof (struct usbdev *));
		devs->dev[devs->ndev++] = dev;
	}
	free(ents);
	devs->current = 0;
	return devs;
}

PUBLIC struct altos_list *
altos_ftdi_list_start(void)
{
	return altos_list_start();
}

int
altos_list_next(struct altos_list *list, struct altos_device *device)
{
	struct altos_usbdev *dev;
	if (list->current >= list->ndev) {
		return 0;
	}
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

struct altos_bt_list {
	inquiry_info	*ii;
	int		sock;
	int		dev_id;
	int		rsp;
	int		num_rsp;
};

#define INQUIRY_MAX_RSP	255

struct altos_bt_list *
altos_bt_list_start(int inquiry_time)
{
	struct altos_bt_list	*bt_list;

	bt_list = calloc(1, sizeof (struct altos_bt_list));
	if (!bt_list)
		goto no_bt_list;

	bt_list->ii = calloc(INQUIRY_MAX_RSP, sizeof (inquiry_info));
	if (!bt_list->ii)
		goto no_ii;
	bt_list->dev_id = hci_get_route(NULL);
	if (bt_list->dev_id < 0)
		goto no_dev_id;

	bt_list->sock = hci_open_dev(bt_list->dev_id);
	if (bt_list->sock < 0)
		goto no_sock;

	bt_list->num_rsp = hci_inquiry(bt_list->dev_id,
				       inquiry_time,
				       INQUIRY_MAX_RSP,
				       NULL,
				       &bt_list->ii,
				       IREQ_CACHE_FLUSH);
	if (bt_list->num_rsp < 0)
		goto no_rsp;

	bt_list->rsp = 0;
	return bt_list;

no_rsp:
	close(bt_list->sock);
no_sock:
no_dev_id:
	free(bt_list->ii);
no_ii:
	free(bt_list);
no_bt_list:
	return NULL;
}

int
altos_bt_list_next(struct altos_bt_list *bt_list,
		   struct altos_bt_device *device)
{
	inquiry_info	*ii;

	if (bt_list->rsp >= bt_list->num_rsp)
		return 0;

	ii = &bt_list->ii[bt_list->rsp];
	ba2str(&ii->bdaddr, device->addr);
	memset(&device->name, '\0', sizeof (device->name));
	if (hci_read_remote_name(bt_list->sock, &ii->bdaddr,
				 sizeof (device->name),
				 device->name, 0) < 0) {
		strcpy(device->name, "[unknown]");
	}
	bt_list->rsp++;
	return 1;
}

void
altos_bt_list_finish(struct altos_bt_list *bt_list)
{
	close(bt_list->sock);
	free(bt_list->ii);
	free(bt_list);
}

void
altos_bt_fill_in(char *name, char *addr, struct altos_bt_device *device)
{
	strncpy(device->name, name, sizeof (device->name));
	device->name[sizeof(device->name)-1] = '\0';
	strncpy(device->addr, addr, sizeof (device->addr));
	device->addr[sizeof(device->addr)-1] = '\0';
}

struct altos_file *
altos_bt_open(struct altos_bt_device *device)
{
	struct sockaddr_rc addr = { 0 };
	int	status, i;
	struct altos_file *file;

	file = calloc(1, sizeof (struct altos_file));
	if (!file)
		goto no_file;
	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = 1;
	str2ba(device->addr, &addr.rc_bdaddr);

	for (i = 0; i < 5; i++) {
		file->fd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
		if (file->fd < 0) {
			altos_set_last_posix_error();
			goto no_sock;
		}

		status = connect(file->fd,
				 (struct sockaddr *)&addr,
				 sizeof(addr));
		if (status >= 0 || errno != EBUSY)
			break;
		close(file->fd);
		usleep(100 * 1000);
	}
	if (status < 0) {
		altos_set_last_posix_error();
		goto no_link;
	}
	usleep(100 * 1000);

#ifdef USE_POLL
	pipe(file->pipe);
#else
	file->out_fd = dup(file->fd);
#endif
	return file;
no_link:
	close(file->fd);
no_sock:
	free(file);
no_file:
	return NULL;
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
	int ftdi;
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

static int
get_number(io_object_t object, CFStringRef entry, int *result)
{
	CFTypeRef entry_as_number;
	Boolean got_number;

	entry_as_number = IORegistryEntrySearchCFProperty (object,
							   kIOServicePlane,
							   entry,
							   kCFAllocatorDefault,
							   kIORegistryIterateRecursively);
	if (entry_as_number) {
		got_number = CFNumberGetValue(entry_as_number,
					      kCFNumberIntType,
					      result);
		if (got_number)
			return 1;
	}
	return 0;
}

PUBLIC struct altos_list *
altos_list_start(void)
{
	struct altos_list *list = calloc (sizeof (struct altos_list), 1);
	CFMutableDictionaryRef matching_dictionary = IOServiceMatching("IOUSBDevice");
	io_iterator_t tdIterator;
	io_object_t tdObject;
	kern_return_t ret;
	int i;

	ret = IOServiceGetMatchingServices(kIOMasterPortDefault, matching_dictionary, &list->iterator);
	if (ret != kIOReturnSuccess) {
		free(list);
		return NULL;
	}
	list->ftdi = 0;
	return list;
}

PUBLIC struct altos_list *
altos_ftdi_list_start(void)
{
	struct altos_list *list = altos_list_start();

	if (list)
		list->ftdi = 1;
	return list;
}

PUBLIC int
altos_list_next(struct altos_list *list, struct altos_device *device)
{
	io_object_t object;
	char serial_string[128];

	for (;;) {
		object = IOIteratorNext(list->iterator);
		if (!object)
			return 0;

		if (!get_number (object, CFSTR(kUSBVendorID), &device->vendor) ||
		    !get_number (object, CFSTR(kUSBProductID), &device->product))
			continue;
		if (get_string (object, CFSTR("IOCalloutDevice"), device->path, sizeof (device->path)) &&
		    get_string (object, CFSTR("USB Product Name"), device->name, sizeof (device->name)) &&
		    get_string (object, CFSTR("USB Serial Number"), serial_string, sizeof (serial_string))) {
			device->serial = atoi(serial_string);
			return 1;
		}
	}
}

PUBLIC void
altos_list_finish(struct altos_list *list)
{
	IOObjectRelease (list->iterator);
	free(list);
}

struct altos_bt_list {
	int		sock;
	int		dev_id;
	int		rsp;
	int		num_rsp;
};

#define INQUIRY_MAX_RSP	255

struct altos_bt_list *
altos_bt_list_start(int inquiry_time)
{
	return NULL;
}

int
altos_bt_list_next(struct altos_bt_list *bt_list,
		   struct altos_bt_device *device)
{
	return 0;
}

void
altos_bt_list_finish(struct altos_bt_list *bt_list)
{
}

void
altos_bt_fill_in(char *name, char *addr, struct altos_bt_device *device)
{
	strncpy(device->name, name, sizeof (device->name));
	device->name[sizeof(device->name)-1] = '\0';
	strncpy(device->addr, addr, sizeof (device->addr));
	device->addr[sizeof(device->addr)-1] = '\0';
}

struct altos_file *
altos_bt_open(struct altos_bt_device *device)
{
	return NULL;
}

#endif


#ifdef WINDOWS

#include <stdlib.h>
#include <windows.h>
#include <setupapi.h>

struct altos_list {
	HDEVINFO	dev_info;
	int		index;
	int		ftdi;
};

#define USB_BUF_SIZE	64

struct altos_file {
	HANDLE				handle;
	unsigned char			out_data[USB_BUF_SIZE];
	int				out_used;
	unsigned char			in_data[USB_BUF_SIZE];
	int				in_used;
	int				in_read;
	OVERLAPPED			ov_read;
	BOOL				pend_read;
	OVERLAPPED			ov_write;
};

#include <stdarg.h>

static void
log_message(char *fmt, ...)
{
	static	FILE *log = NULL;
	va_list	a;

	if (!log)
		log = fopen("\\temp\\altos.txt", "w");
	if (log) {
		SYSTEMTIME time;
		GetLocalTime(&time);
		fprintf (log, "%4d-%02d-%02d %2d:%02d:%02d. ",
			 time.wYear, time.wMonth, time.wDay,
			 time.wHour, time.wMinute, time.wSecond);
		va_start(a, fmt);
		vfprintf(log, fmt, a);
		va_end(a);
		fflush(log);
	}
}

static void
_altos_set_last_windows_error(char *file, int line)
{
	DWORD	error = GetLastError();
	TCHAR	message[1024];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
		      0,
		      error,
		      0,
		      message,
		      sizeof (message) / sizeof (TCHAR),
		      NULL);
	if (error != ERROR_SUCCESS)
		log_message ("%s:%d %s\n", file, line, message);
	altos_set_last_error(error, message);
}

#define altos_set_last_windows_error() _altos_set_last_windows_error(__FILE__, __LINE__)

PUBLIC struct altos_list *
altos_list_start(void)
{
	struct altos_list	*list = calloc(1, sizeof (struct altos_list));

	if (!list)
		return NULL;
	list->dev_info = SetupDiGetClassDevs(NULL, "USB", NULL,
					     DIGCF_ALLCLASSES|DIGCF_PRESENT);
	if (list->dev_info == INVALID_HANDLE_VALUE) {
		altos_set_last_windows_error();
		free(list);
		return NULL;
	}
	list->index = 0;
	list->ftdi = 0;
	return list;
}

PUBLIC struct altos_list *
altos_ftdi_list_start(void)
{
	struct altos_list	*list = calloc(1, sizeof (struct altos_list));

	if (!list)
		return NULL;
	list->dev_info = SetupDiGetClassDevs(NULL, "FTDIBUS", NULL,
					     DIGCF_ALLCLASSES|DIGCF_PRESENT);
	if (list->dev_info == INVALID_HANDLE_VALUE) {
		altos_set_last_windows_error();
		free(list);
		return NULL;
	}
	list->index = 0;
	list->ftdi = 1;
	return list;
}

PUBLIC int
altos_list_next(struct altos_list *list, struct altos_device *device)
{
	SP_DEVINFO_DATA dev_info_data;
	BYTE		port[128];
	DWORD		port_len;
	char		friendlyname[256];
	BYTE		symbolic[256];
	DWORD		symbolic_len;
	HKEY		dev_key;
	unsigned int	vid, pid;
	int		serial;
	HRESULT 	result;
	DWORD		friendlyname_type;
	DWORD		friendlyname_len;

	dev_info_data.cbSize = sizeof (SP_DEVINFO_DATA);
	while(SetupDiEnumDeviceInfo(list->dev_info, list->index,
				    &dev_info_data))
	{
		list->index++;

		dev_key = SetupDiOpenDevRegKey(list->dev_info, &dev_info_data,
					       DICS_FLAG_GLOBAL, 0, DIREG_DEV,
					       KEY_READ);
		if (dev_key == INVALID_HANDLE_VALUE) {
			altos_set_last_windows_error();
			continue;
		}

		if (list->ftdi) {
			vid = 0x0403;
			pid = 0x6015;
			serial = 0;
		} else {
			/* Fetch symbolic name for this device and parse out
			 * the vid/pid/serial info */
			symbolic_len = sizeof(symbolic);
			result = RegQueryValueEx(dev_key, "SymbolicName", NULL, NULL,
						 symbolic, &symbolic_len);
			if (result != 0) {
				altos_set_last_windows_error();
				RegCloseKey(dev_key);
				continue;
			}
			vid = pid = serial = 0;
			sscanf((char *) symbolic + sizeof("\\??\\USB#VID_") - 1,
			       "%04X", &vid);
			sscanf((char *) symbolic + sizeof("\\??\\USB#VID_XXXX&PID_") - 1,
			       "%04X", &pid);
			sscanf((char *) symbolic + sizeof("\\??\\USB#VID_XXXX&PID_XXXX#") - 1,
			       "%d", &serial);
		}

		/* Fetch the com port name */
		port_len = sizeof (port);
		result = RegQueryValueEx(dev_key, "PortName", NULL, NULL,
					 port, &port_len);
		RegCloseKey(dev_key);
		if (result != 0) {
			altos_set_last_windows_error();
			continue;
		}

		/* Fetch the device description which is the device name,
		 * with firmware that has unique USB ids */
		friendlyname_len = sizeof (friendlyname);
		if(!SetupDiGetDeviceRegistryProperty(list->dev_info,
						     &dev_info_data,
						     SPDRP_FRIENDLYNAME,
						     &friendlyname_type,
						     (BYTE *)friendlyname,
						     sizeof(friendlyname),
						     &friendlyname_len))
		{
			altos_set_last_windows_error();
			continue;
		}
		device->vendor = vid;
		device->product = pid;
		device->serial = serial;
		strcpy(device->name, friendlyname);

		strcpy(device->path, (char *) port);
		return 1;
	}
	result = GetLastError();
	if (result != ERROR_NO_MORE_ITEMS)
		altos_set_last_windows_error();
	return 0;
}

PUBLIC void
altos_list_finish(struct altos_list *list)
{
	SetupDiDestroyDeviceInfoList(list->dev_info);
	free(list);
}

static int
altos_queue_read(struct altos_file *file)
{
	DWORD	got;
	if (file->pend_read)
		return LIBALTOS_SUCCESS;

	if (!ReadFile(file->handle, file->in_data, USB_BUF_SIZE, &got, &file->ov_read)) {
		if (GetLastError() != ERROR_IO_PENDING) {
			altos_set_last_windows_error();
			return LIBALTOS_ERROR;
		}
		file->pend_read = TRUE;
	} else {
		file->pend_read = FALSE;
		file->in_read = 0;
		file->in_used = got;
	}
	return LIBALTOS_SUCCESS;
}

static int
altos_wait_read(struct altos_file *file, int timeout)
{
	DWORD	ret;
	DWORD	got;

	if (!file->pend_read)
		return LIBALTOS_SUCCESS;

	if (!timeout)
		timeout = INFINITE;

	ret = WaitForSingleObject(file->ov_read.hEvent, timeout);
	switch (ret) {
	case WAIT_OBJECT_0:
		if (!GetOverlappedResult(file->handle, &file->ov_read, &got, FALSE)) {
			altos_set_last_windows_error();
			return LIBALTOS_ERROR;
		}
		file->pend_read = FALSE;
		file->in_read = 0;
		file->in_used = got;
		break;
	case WAIT_TIMEOUT:
		return LIBALTOS_TIMEOUT;
		break;
	default:
		altos_set_last_windows_error();
		return LIBALTOS_ERROR;
	}
	return LIBALTOS_SUCCESS;
}

static int
altos_fill(struct altos_file *file, int timeout)
{
	int	ret;

	if (file->in_read < file->in_used)
		return LIBALTOS_SUCCESS;

	file->in_read = file->in_used = 0;

	ret = altos_queue_read(file);
	if (ret)
		return ret;
	ret = altos_wait_read(file, timeout);
	if (ret)
		return ret;

	return LIBALTOS_SUCCESS;
}

PUBLIC int
altos_flush(struct altos_file *file)
{
	DWORD		put;
	unsigned char	*data = file->out_data;
	int		used = file->out_used;
	DWORD		ret;

	while (used) {
		if (!WriteFile(file->handle, data, used, &put, &file->ov_write)) {
			if (GetLastError() != ERROR_IO_PENDING) {
				altos_set_last_windows_error();
				return LIBALTOS_ERROR;
			}
			ret = WaitForSingleObject(file->ov_write.hEvent, INFINITE);
			switch (ret) {
			case WAIT_OBJECT_0:
				if (!GetOverlappedResult(file->handle, &file->ov_write, &put, FALSE)) {
					altos_set_last_windows_error();
					return LIBALTOS_ERROR;
				}
				break;
			default:
				altos_set_last_windows_error();
				return LIBALTOS_ERROR;
			}
		}
		data += put;
		used -= put;
	}
	file->out_used = 0;
	return LIBALTOS_SUCCESS;
}

static HANDLE
open_serial(char *full_name)
{
	HANDLE	handle;
	DCB	dcb;

	handle = CreateFile(full_name, GENERIC_READ|GENERIC_WRITE,
			    0, NULL, OPEN_EXISTING,
			    FILE_FLAG_OVERLAPPED, NULL);

	if (handle == INVALID_HANDLE_VALUE) {
		altos_set_last_windows_error();
		return INVALID_HANDLE_VALUE;
	}

	if (!GetCommState(handle, &dcb)) {
		altos_set_last_windows_error();
		CloseHandle(handle);
		return INVALID_HANDLE_VALUE;
	}
	dcb.BaudRate = CBR_9600;
	dcb.fBinary = TRUE;
	dcb.fParity = FALSE;
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fDtrControl = DTR_CONTROL_ENABLE;
	dcb.fDsrSensitivity = FALSE;
	dcb.fTXContinueOnXoff = FALSE;
	dcb.fOutX = FALSE;
	dcb.fInX = FALSE;
	dcb.fErrorChar = FALSE;
	dcb.fNull = FALSE;
	dcb.fRtsControl = RTS_CONTROL_ENABLE;
	dcb.fAbortOnError = FALSE;
	dcb.XonLim = 10;
	dcb.XoffLim = 10;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	dcb.XonChar = 17;
	dcb.XoffChar = 19;
#if 0
	dcb.ErrorChar = 0;
	dcb.EofChar = 0;
	dcb.EvtChar = 0;
#endif
	if (!SetCommState(handle, &dcb)) {
		altos_set_last_windows_error();
		CloseHandle(handle);
		return INVALID_HANDLE_VALUE;
	}
	return handle;
}

PUBLIC struct altos_file *
altos_open(struct altos_device *device)
{
	struct altos_file	*file = calloc (1, sizeof (struct altos_file));
	char	full_name[64];
	COMMTIMEOUTS timeouts;
	int i;

	if (!file)
		return NULL;

	strcpy(full_name, "\\\\.\\");
	strcat(full_name, device->path);

	file->handle = INVALID_HANDLE_VALUE;

	for (i = 0; i < 5; i++) {
		file->handle = open_serial(full_name);
		if (file->handle != INVALID_HANDLE_VALUE)
			break;
		altos_set_last_windows_error();
		Sleep(100);
	}

	if (file->handle == INVALID_HANDLE_VALUE) {
		free(file);
		return NULL;
	}

	/* The FTDI driver doesn't appear to work right unless you open it twice */
	if (device->vendor == 0x0403) {
		CloseHandle(file->handle);
		file->handle = open_serial(full_name);
		if (file->handle == INVALID_HANDLE_VALUE) {
			free(file);
			return NULL;
		}
	}

	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
	timeouts.ReadTotalTimeoutConstant = 1 << 30;	/* almost forever */
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;
	SetCommTimeouts(file->handle, &timeouts);

	file->ov_read.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	file->ov_write.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	return file;
}

PUBLIC void
altos_close(struct altos_file *file)
{
	HANDLE	handle = file->handle;
	if (handle != INVALID_HANDLE_VALUE) {
		HANDLE	ov_read = file->ov_read.hEvent;
		HANDLE	ov_write = file->ov_write.hEvent;
		file->handle = INVALID_HANDLE_VALUE;
		file->ov_read.hEvent = INVALID_HANDLE_VALUE;
		file->ov_write.hEvent = INVALID_HANDLE_VALUE;
		PurgeComm(handle, PURGE_RXABORT|PURGE_RXCLEAR|PURGE_TXABORT|PURGE_TXCLEAR);
		Sleep(100);
		CloseHandle(handle);
		file->handle = INVALID_HANDLE_VALUE;
		CloseHandle(ov_read);
		CloseHandle(ov_write);
	}
}

PUBLIC void
altos_free(struct altos_file *file)
{
	altos_close(file);
	free(file);
}

PUBLIC int
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

PUBLIC int
altos_getchar(struct altos_file *file, int timeout)
{
	int	ret;
	while (file->in_read == file->in_used) {
		if (file->handle == INVALID_HANDLE_VALUE) {
			altos_set_last_windows_error();
			return LIBALTOS_ERROR;
		}
		ret = altos_fill(file, timeout);
		if (ret)
			return ret;
	}
	return file->in_data[file->in_read++];
}

struct altos_bt_list *
altos_bt_list_start(int inquiry_time)
{
	return NULL;
}

int
altos_bt_list_next(struct altos_bt_list *bt_list,
		   struct altos_bt_device *device)
{
	return 0;
}

void
altos_bt_list_finish(struct altos_bt_list *bt_list)
{
	free(bt_list);
}

void
altos_bt_fill_in(char *name, char *addr, struct altos_bt_device *device)
{
	strncpy(device->name, name, sizeof (device->name));
	device->name[sizeof(device->name)-1] = '\0';
	strncpy(device->addr, addr, sizeof (device->addr));
	device->addr[sizeof(device->addr)-1] = '\0';
}

struct altos_file *
altos_bt_open(struct altos_bt_device *device)
{
	return NULL;
}

#endif

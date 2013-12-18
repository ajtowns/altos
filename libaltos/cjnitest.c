#include <stdio.h>
#include "libaltos.h"

static void
altos_puts(struct altos_file *file, char *string)
{
	char	c;

	while ((c = *string++))
		altos_putchar(file, c);
}

int
main (int argc, char **argv)
{
	struct altos_device	device;
	struct altos_list	*list;
	struct altos_bt_device	bt_device;
	struct altos_bt_list	*bt_list;

	altos_init();
	list = altos_list_start();
	while (altos_list_next(list, &device)) {
		struct altos_file	*file;
		int			c;

		printf ("%04x:%04x %-20s %4d %s\n", device.vendor, device.product,
			device.name, device.serial, device.path);

		file = altos_open(&device);
		if (!file) {
			printf("altos_open failed\n");
			continue;
		}
		altos_puts(file,"v\nc s\n");
		altos_flush(file);
		while ((c = altos_getchar(file, 100)) >= 0) {
			putchar (c);
		}
		if (c != LIBALTOS_TIMEOUT)
			printf ("getchar returns %d\n", c);
		altos_close(file);
	}
	altos_list_finish(list);
#if HAS_BLUETOOTH
	bt_list = altos_bt_list_start(8);
	while (altos_bt_list_next(bt_list, &bt_device)) {
		printf ("%s %s\n", bt_device.name, bt_device.addr);
		if (strncmp(bt_device.name, "TeleBT", 6) == 0) {
			struct altos_file	*file;

			int			c;
			file = altos_bt_open(&bt_device);
			if (!file) {
				printf("altos_bt_open failed\n");
				continue;
			}
			altos_puts(file,"v\nc s\n");
			altos_flush(file);
			while ((c = altos_getchar(file, 100)) >= 0) {
				putchar(c);
			}
			if (c != LIBALTOS_TIMEOUT)
				printf("getchar returns %d\n", c);
			altos_close(file);
		}
	}
	altos_bt_list_finish(bt_list);
#endif
	altos_fini();
	return 0;
}

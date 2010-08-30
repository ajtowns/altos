#include <stdio.h>
#include "libaltos.h"

static void
altos_puts(struct altos_file *file, char *string)
{
	char	c;

	while ((c = *string++))
		altos_putchar(file, c);
}

main ()
{
	struct altos_device	device;
	struct altos_list	*list;

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
	altos_fini();
}

#include <stdio.h>
#include "libaltos.h"

main ()
{
	struct altos_device	device;
	struct altos_list	*list;

	altos_init();
	list = altos_list_start();
	while (altos_list_next(list, &device)) {
		struct altos_file	*file;
		int			c;

		file = altos_open(&device);
		altos_putchar(file, '?'); altos_putchar(file, '\n'); altos_flush(file);
		while ((c = altos_getchar(file, 100)) >= 0) {
			putchar (c);
		}
		printf ("getchar returns %d\n", c);
		altos_close(file);
	}
	altos_list_finish(list);
	altos_fini();
}

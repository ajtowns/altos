bin_PROGRAMS=ao-sky-flash

AM_CFLAGS=-I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)
AO_SKY_FLASH_LIBS=$(top_builddir)/ao-tools/lib/libao-tools.a

ao_sky_flash_DEPENDENCIES = $(AO_SKY_FLASH_LIBS)

ao_sky_flash_LDADD=$(AO_SKY_FLASH_LIBS) $(LIBUSB_LIBS)

ao_sky_flash_SOURCES = \
	sky_bin.c \
	sky_debug.c \
	sky_flash.c \
	sky_flash.h \
	sky_serial.c \
	sky_srec.c

man_MANS = ao-sky-flash.1

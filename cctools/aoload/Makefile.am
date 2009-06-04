bin_PROGRAMS=aoload

AM_CFLAGS=-I$(top_srcdir)/cctools/lib $(LIBUSB_CFLAGS)
AOLOAD_LIBS=$(top_builddir)/cctools/lib/libcc.a

aoload_DEPENDENCIES = $(AOLOAD_LIBS)

aoload_LDADD=$(AOLOAD_LIBS) $(LIBUSB_LIBS)

aoload_SOURCES = aoload.c

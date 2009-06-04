bin_PROGRAMS=ccload

AM_CFLAGS=-I$(top_srcdir)/cctools/lib $(LIBUSB_CFLAGS)
CCLOAD_LIBS=$(top_builddir)/cctools/lib/libcc.a

ccload_DEPENDENCIES = $(CCLOAD_LIBS)

ccload_LDADD=$(CCLOAD_LIBS) $(LIBUSB_LIBS)

ccload_SOURCES = ccload.c

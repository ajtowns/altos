bin_PROGRAMS=ccmanual

AM_CFLAGS=-I$(top_srcdir)/cctools/lib $(LIBUSB_CFLAGS)
CCMANUAL_LIBS=$(top_builddir)/cctools/lib/libcc.a

ccmanual_DEPENDENCIES = $(CCMANUAL_LIBS)

ccmanual_LDADD=$(CCMANUAL_LIBS) $(LIBUSB_LIBS)

ccmanual_SOURCES = ccmanual.c

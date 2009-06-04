bin_PROGRAMS=ccdump

AM_CFLAGS=-I$(top_srcdir)/cctools/lib $(LIBUSB_CFLAGS)
CCDUMP_LIBS=$(top_builddir)/cctools/lib/libcc.a

ccdump_DEPENDENCIES = $(CCDUMP_LIBS)

ccdump_LDADD=$(CCDUMP_LIBS) $(LIBUSB_LIBS)

ccdump_SOURCES = ccdump.c

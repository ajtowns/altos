bin_PROGRAMS=ccdump

AM_CFLAGS=-I$(top_srcdir)/lib $(LIBUSB_CFLAGS)
CCLOAD_LIBS=../lib/libcc.a

ccdump_DEPENDENCIES = $(CCLOAD_LIBS)

ccdump_LDADD=$(CCLOAD_LIBS) $(LIBUSB_LIBS)

ccdump_SOURCES = ccdump.c

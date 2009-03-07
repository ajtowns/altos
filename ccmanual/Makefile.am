bin_PROGRAMS=ccmanual

AM_CFLAGS=-I$(top_srcdir)/lib $(LIBUSB_CFLAGS)
CCMANUAL_LIBS=../lib/libcc.a

ccmanual_DEPENDENCIES = $(CCMANUAL_LIBS)

ccmanual_LDADD=$(CCMANUAL_LIBS) $(LIBUSB_LIBS)

ccmanual_SOURCES = ccmanual.c

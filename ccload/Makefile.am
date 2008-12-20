bin_PROGRAMS=ccload

AM_CFLAGS=-I$(top_srcdir)/lib
CCLOAD_LIBS=../lib/libcc.a

ccload_DEPENDENCIES = $(CCLOAD_LIBS)

ccload_LDADD=$(CCLOAD_LIBS) $(USB_LIBS)

ccload_SOURCES = ccload.c

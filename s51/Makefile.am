bin_PROGRAMS=s51

AM_CFLAGS=-I$(top_srcdir)/lib $(LIBUSB_CFLAGS)
S51_LIBS=../lib/libcc.a

man_MANS = s51.1

s51_DEPENDENCIES = $(S51_LIBS)

s51_LDADD=$(S51_LIBS) $(LIBUSB_LIBS)

s51_SOURCES = s51-parse.c s51-command.c s51-main.c

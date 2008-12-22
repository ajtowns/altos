bin_PROGRAMS=s51

AM_CFLAGS=-I$(top_srcdir)/lib
S51_LIBS=../lib/libcc.a

s51_DEPENDENCIES = $(S51_LIBS)

s51_LDADD=$(S51_LIBS) $(USB_LIBS)

s51_SOURCES = s51-parse.c s51-command.c s51-main.c

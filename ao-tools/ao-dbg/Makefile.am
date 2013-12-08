bin_PROGRAMS=ao-dbg

AM_CFLAGS=-I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)
AO_DBG_LIBS=$(top_builddir)/ao-tools/lib/libao-tools.a

man_MANS = ao-dbg.1

ao_dbg_DEPENDENCIES = $(AO_DBG_LIBS)

ao_dbg_LDADD=$(AO_DBG_LIBS) $(LIBUSB_LIBS) $(LIBREADLINE)

ao_dbg_SOURCES = ao-dbg-parse.c ao-dbg-command.c ao-dbg-main.c

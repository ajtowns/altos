bin_PROGRAMS=ao-dumplog

AM_CFLAGS=-I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS) $(GNOME_CFLAGS)
AO_DUMPLOG_LIBS=$(top_builddir)/ao-tools/lib/libao-tools.a

ao_dumplog_DEPENDENCIES = $(AO_DUMPLOG_LIBS)

ao_dumplog_LDADD=$(AO_DUMPLOG_LIBS) $(LIBUSB_LIBS) $(GNOME_LIBS)

ao_dumplog_SOURCES = ao-dumplog.c

man_MANS = ao-dumplog.1

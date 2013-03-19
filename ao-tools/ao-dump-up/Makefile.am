bin_PROGRAMS=ao-dump-up

AM_CFLAGS=-I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS) $(GNOME_CFLAGS)
AO_DUMP_LOG_LIBS=$(top_builddir)/ao-tools/lib/libao-tools.a

ao_dump_up_DEPENDENCIES = $(AO_DUMP_LOG_LIBS)

ao_dump_up_LDADD=$(AO_DUMP_LOG_LIBS) $(LIBUSB_LIBS) $(GNOME_LIBS)

ao_dump_up_SOURCES = ao-dump-up.c

man_MANS = ao-dump-up.1

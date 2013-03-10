bin_PROGRAMS=ao-dumpflash

AM_CFLAGS=-I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)
AO_DUMPLOG_LIBS=$(top_builddir)/ao-tools/lib/libao-tools.a

ao_dumpflash_DEPENDENCIES = $(AO_DUMPLOG_LIBS)

ao_dumpflash_LDADD=$(AO_DUMPLOG_LIBS) $(LIBUSB_LIBS)

ao_dumpflash_SOURCES = ao-dumpflash.c

man_MANS = ao-dumpflash.1

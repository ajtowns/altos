if LIBSTLINK

bin_PROGRAMS=ao-elftohex

LIBSTLINKDIR=/local/src/stlink

AM_CFLAGS=-I$(top_srcdir)/ao-tools/lib $(LIBSTLINK_CFLAGS) $(LIBUSB_CFLAGS)
AO_STMLOAD_LIBS=$(top_builddir)/ao-tools/lib/libao-tools.a

ao_elftohex_DEPENDENCIES = $(AO_STMLOAD_LIBS)

ao_elftohex_LDADD=$(AO_STMLOAD_LIBS) $(LIBSTLINK_LIBS) $(LIBUSB_LIBS) -lelf

ao_elftohex_SOURCES=ao-elftohex.c

man_MANS = ao-elftohex.1

endif

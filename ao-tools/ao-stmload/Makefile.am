if LIBSTLINK

bin_PROGRAMS=ao-stmload

AM_CFLAGS=-I$(top_srcdir)/ao-tools/lib $(STLINK_CFLAGS) $(LIBUSB_CFLAGS)
AO_STMLOAD_LIBS=$(top_builddir)/ao-tools/lib/libao-tools.a

ao_stmload_DEPENDENCIES = $(AO_STMLOAD_LIBS)

ao_stmload_LDADD=$(AO_STMLOAD_LIBS) $(STLINK_LIBS) $(LIBUSB_LIBS) -lelf

ao_stmload_SOURCES=ao-stmload.c ao-stmload.h

man_MANS = ao-stmload.1

endif

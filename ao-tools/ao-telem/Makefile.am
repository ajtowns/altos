bin_PROGRAMS=ao-telem

AM_CFLAGS=-I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)
AO_POSTFLIGHT_LIBS=$(top_builddir)/ao-tools/lib/libao-tools.a

ao_telem_DEPENDENCIES = $(AO_POSTFLIGHT_LIBS)

ao_telem_LDADD=$(AO_POSTFLIGHT_LIBS) $(LIBUSB_LIBS)

ao_telem_SOURCES = ao-telem.c

man_MANS = ao-telem.1

bin_PROGRAMS=ao-send-telem

AM_CFLAGS=-I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)
AO_POSTFLIGHT_LIBS=$(top_builddir)/ao-tools/lib/libao-tools.a

ao_send_telem_DEPENDENCIES = $(AO_POSTFLIGHT_LIBS)

ao_send_telem_LDADD=$(AO_POSTFLIGHT_LIBS) $(LIBUSB_LIBS)

ao_send_telem_SOURCES = ao-send-telem.c

man_MANS = ao-send-telem.1

bin_PROGRAMS=ao-postflight

AM_CFLAGS=-I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS) $(GNOME_CFLAGS) $(PLPLOT_CFLAGS)
AO_POSTFLIGHT_LIBS=$(top_builddir)/ao-tools/lib/libao-tools.a

ao_postflight_DEPENDENCIES = $(AO_POSTFLIGHT_LIBS)

ao_postflight_LDADD=$(AO_POSTFLIGHT_LIBS) $(LIBUSB_LIBS) $(GNOME_LIBS) $(PLPLOT_LIBS)

ao_postflight_SOURCES = ao-postflight.c

man_MANS = ao-postflight.1

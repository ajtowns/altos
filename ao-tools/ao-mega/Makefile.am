bin_PROGRAMS=ao-mega

AM_CFLAGS=-I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)
AO_POSTFLIGHT_LIBS=$(top_builddir)/ao-tools/lib/libao-tools.a

ao_mega_DEPENDENCIES = $(AO_POSTFLIGHT_LIBS)

ao_mega_LDADD=$(AO_POSTFLIGHT_LIBS) $(LIBUSB_LIBS)

ao_mega_SOURCES = ao-mega.c

man_MANS = ao-mega.1

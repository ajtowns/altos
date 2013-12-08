bin_PROGRAMS=ao-elftohex

AM_CFLAGS=-I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)
AO_ELFTOHEX_LIBS=$(top_builddir)/ao-tools/lib/libao-tools.a

ao_elftohex_DEPENDENCIES = $(AO_ELFTOHEX_LIBS)

ao_elftohex_LDADD=$(AO_ELFTOHEX_LIBS) -lelf

ao_elftohex_SOURCES=ao-elftohex.c

man_MANS = ao-elftohex.1

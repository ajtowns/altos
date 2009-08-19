bin_PROGRAMS=ao-rawload

AM_CFLAGS=-I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)
AO_RAWLOAD_LIBS=$(top_builddir)/ao-tools/lib/libao-tools.a

ao_rawload_DEPENDENCIES = $(AO_RAWLOAD_LIBS)

ao_rawload_LDADD=$(AO_RAWLOAD_LIBS) $(LIBUSB_LIBS)

ao_rawload_SOURCES = ao-rawload.c

man_MANS=ao-rawload.1

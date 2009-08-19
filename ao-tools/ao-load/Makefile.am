bin_PROGRAMS=ao-load

AM_CFLAGS=-I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)
AO_LOAD_LIBS=$(top_builddir)/ao-tools/lib/libao-tools.a

ao_load_DEPENDENCIES = $(AO_LOAD_LIBS)

ao_load_LDADD=$(AO_LOAD_LIBS) $(LIBUSB_LIBS)

ao_load_SOURCES = ao-load.c

man_MANS = ao-load.1

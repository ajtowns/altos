bin_PROGRAMS=ao-bitbang

AM_CFLAGS=-I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)
AO_BITBANG_LIBS=$(top_builddir)/ao-tools/lib/libao-tools.a

ao_bitbang_DEPENDENCIES = $(AO_BITBANG_LIBS)

ao_bitbang_LDADD=$(AO_BITBANG_LIBS) $(LIBUSB_LIBS)

ao_bitbang_SOURCES = ao-bitbang.c

man_MANS=ao-bitbang.1

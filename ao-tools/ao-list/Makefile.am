bin_PROGRAMS=ao-list

AM_CFLAGS=-I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)
AO_LIST_LIBS=$(top_builddir)/ao-tools/lib/libao-tools.a

ao_list_DEPENDENCIES = $(AO_LIST_LIBS)

ao_list_LDADD=$(AO_LIST_LIBS) $(LIBUSB_LIBS)

ao_list_SOURCES = ao-list.c

man_MANS = ao-list.1

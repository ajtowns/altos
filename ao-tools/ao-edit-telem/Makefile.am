bin_PROGRAMS=ao-edit-telem

AM_CFLAGS=-I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)
AO_POSTFLIGHT_LIBS=$(top_builddir)/ao-tools/lib/libao-tools.a

ao_edit_telem_DEPENDENCIES = $(AO_POSTFLIGHT_LIBS)

ao_edit_telem_LDADD=$(AO_POSTFLIGHT_LIBS) $(LIBUSB_LIBS)

ao_edit_telem_SOURCES = ao-edit-telem.c

man_MANS = ao-edit-telem.1

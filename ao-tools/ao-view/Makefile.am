VERSION=$(shell git describe)

AO_VIEW_CFLAGS=-I$(top_srcdir)/ao-tools/lib
AO_VIEW_LIBS=$(top_builddir)/ao-tools/lib/libao-tools.a
AM_CFLAGS=$(AO_VIEW_CFLAGS) $(GNOME_CFLAGS) $(ALSA_CFLAGS) -I$(top_srcdir)/src -DAOVIEW_VERSION=\"$(VERSION)\" @FLITE_INCS@

bin_PROGRAMS=ao-view

ao_view_DEPENDENCIES=$(AO_VIEW_LIBS)
ao_view_LDADD=$(GNOME_LIBS) $(FLITE_LIBS) $(ALSA_LIBS) $(AO_VIEW_LIBS) $(LIBUSB_LIBS)

ao_view_SOURCES = \
	aoview_main.c \
	aoview_dev_dialog.c \
	aoview_serial.c \
	aoview_monitor.c \
	aoview_state.c \
	aoview_convert.c \
	aoview_log.c \
	aoview_table.c \
	aoview_util.c \
	aoview_file.c \
	aoview_eeprom.c \
	aoview_voice.c \
	aoview_replay.c \
	aoview_label.c \
	aoview_flite.c \
	aoview_channel.c \
	aoview.h

BUILT_SOURCES = aoview_glade.h

CLEANFILES = aoview_glade.h

man_MANS=ao-view.1

aoview_glade.h: aoview.glade
	sed -e 's/"/\\"/g' -e 's/^/"/' -e 's/$$/"/' $< > $@

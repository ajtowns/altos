VERSION=$(shell git describe)
AM_CFLAGS=$(GNOME_CFLAGS) -I$(top_srcdir)/src -DAOVIEW_VERSION=\"$(VERSION)\" @FLITE_INCS@

if USE_FLITE
FLITE_PROG=aoview_flite
endif

bin_PROGRAMS=aoview $(FLITE_PROG) aoview_slowtelem

aoview_LDADD=$(GNOME_LIBS)

aoview_SOURCES = \
	aoview_main.c \
	aoview_dev.c \
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
	aoview.h

aoview_flite_SOURCES = \
	aoview_flite.c

aoview_slowtelem_SOURCES = \
	aoview_slowtelem.c

aoview_flite_LDADD=@FLITE_LIBS@

BUILT_SOURCES = aoview_glade.h

CLEANFILES = aoview_glade.h

aoview_glade.h: aoview.glade
	sed -e 's/"/\\"/g' -e 's/^/"/' -e 's/$$/"/' $< > $@

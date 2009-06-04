VERSION=$(shell git describe)
AM_CFLAGS=$(GNOME_CFLAGS) -I$(top_srcdir)/src -DAOVIEW_VERSION=\"$(VERSION)\"

bin_PROGRAMS=aoview

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
	aoview.h

BUILT_SOURCES = aoview_glade.h

CLEANFILES = aoview_glade.h

aoview_glade.h: aoview.glade
	sed -e 's/"/\\"/g' -e 's/^/"/' -e 's/$$/"/' $< > $@

if LIBSTLINK

bin_PROGRAMS=ao-stmload

LIBSTLINKDIR=/local/src/stlink

AM_CFLAGS=$(LIBSTLINK_CFLAGS) $(LIBUSB_CFLAGS) -I../lib

ao_stmload_LDADD=$(LIBSTLINK_LIBS) $(LIBUSB_LIBS) -lelf

ao_stmload_SOURCES=ao-stmload.c

man_MANS = ao-stmload.1

endif

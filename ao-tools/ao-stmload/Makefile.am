bin_PROGRAMS=ao-stmload

LIBSTLINKDIR=/local/src/stlink

AM_CFLAGS=-I$(LIBSTLINKDIR)/src $(LIBUSB_CFLAGS) -I../lib

AO_STMLOAD_LIBS=$(LIBSTLINKDIR)/libstlink.a
ao_stmload_DEPENDENCIES = $(AO_STMLOAD_LIBS)

ao_stmload_LDADD=$(AO_STMLOAD_LIBS) $(LIBUSB_LIBS) -lelf

ao_stmload_SOURCES=ao-stmload.c

man_MANS = ao-stmload.1

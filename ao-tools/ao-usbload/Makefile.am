bin_PROGRAMS=ao-usbload

AM_CFLAGS=-I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)
AO_STMLOAD_LIBS=$(top_builddir)/ao-tools/lib/libao-tools.a

ao_usbload_DEPENDENCIES = $(AO_STMLOAD_LIBS)

ao_usbload_LDADD=$(AO_STMLOAD_LIBS) $(LIBUSB_LIBS) -lelf

ao_usbload_SOURCES=ao-usbload.c ao-usbload.h

man_MANS = ao-usbload.1

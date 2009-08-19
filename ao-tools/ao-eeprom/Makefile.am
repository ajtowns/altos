bin_PROGRAMS=ao-eeprom

AM_CFLAGS=-I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)
AO_EEPROM_LIBS=$(top_builddir)/ao-tools/lib/libao-tools.a

ao_eeprom_DEPENDENCIES = $(AO_EEPROM_LIBS)

ao_eeprom_LDADD=$(AO_EEPROM_LIBS) $(LIBUSB_LIBS)

ao_eeprom_SOURCES = ao-eeprom.c

man_MANS = ao-eeprom.1

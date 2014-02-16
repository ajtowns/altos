RES=16 32 48 256

AM_FILES=$(shell for i in $(RES); do echo altus-metrum-$$i.png; done)
MP_FILES=$(shell for i in $(RES); do echo micropeak-$$i.png; done)

ICO_FILES=altus-metrum.ico micro-peak.ico

icondir = $(datadir)/icons/hicolor/scalable/apps

AM_ICON = altusmetrum.svg
MP_ICON = micropeak.svg

icon_DATA = $(AM_ICON) $(MP_ICON)

EXTRA_DIST = $(icon_DATA) $(AM_FILES) $(MP_FILES)

all-local: $(ICO_FILES)

#clean-local:
#	$(RM) -f $(ICO_FILES)

altus-metrum.ico: $(AM_FILES)
	icotool -c -o $@ $(AM_FILES)

micro-peak.ico: $(MP_FILES)
	icotool -c -o $@ $(MP_FILES)
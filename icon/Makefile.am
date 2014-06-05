RES=16 32 48 64 128 256

AM_FILES=$(shell for i in $(RES); do echo altus-metrum-$$i.png; done)
MP_FILES=$(shell for i in $(RES); do echo micropeak-$$i.png; done)
TG_FILES=$(shell for i in $(RES); do echo telegps-$$i.png; done)

ICO_FILES=altus-metrum.ico micro-peak.ico telegps.ico

icondir = $(datadir)/icons/hicolor/scalable/apps

AM_ICON = altusmetrum.svg
MP_ICON = micropeak.svg
TG_ICON = telegps.svg

icon_DATA = $(AM_ICON) $(MP_ICON) $(TG_ICON)

EXTRA_DIST = $(icon_DATA) $(AM_FILES) $(MP_FILES) $(TG_FILES)

all-local: $(ICO_FILES)

$(TG_FILES): telegps.svg
	for i in $(RES); do rsvg-convert -w $$i -o telegps-$$i.png telegps.svg; done

#clean-local:
#	$(RM) -f $(ICO_FILES)

altus-metrum.ico: $(AM_FILES)
	icotool -c -o $@ $(AM_FILES)

micro-peak.ico: $(MP_FILES)
	icotool -c -o $@ $(MP_FILES)

telegps.ico: $(TG_FILES)
	icotool -c -o $@ $(TG_FILES)

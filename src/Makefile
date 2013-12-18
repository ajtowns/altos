#
# AltOS build
#

vpath make-altitude util
vpath make-altitude-pa util
vpath make-kalman util
vpath make-whiten util
vpath kalman.5c kalman
vpath kalman_filter.5c kalman
vpath kalman_micro.5c kalman
vpath load_csv.5c kalman
vpath matrix.5c kalman

include Version
include Makedefs

SDCCDIRS=\
	telemetrum-v1.2 telemetrum-v1.1 telemetrum-v1.0 \
	teledongle-v0.2 \
	telemini-v1.0 \
	telebt-v1.0 \
	teleterra-v0.2 teleshield-v0.1 \
	telefire-v0.1 telefire-v0.2 \
	telemini-v2.0

ARMM3DIRS=\
	telemega-v0.1 telemega-v0.1/flash-loader \
	telemega-v1.0 telemega-v1.0/flash-loader \
	telemetrum-v2.0 telemetrum-v2.0/flash-loader \
	megadongle-v0.1 megadongle-v0.1/flash-loader \
	telegps-v0.3 telegps-v0.3/flash-loader \
	telelco-v0.2 telelco-v0.2/flash-loader \
	telescience-v0.2 telescience-v0.2/flash-loader

ARMM0DIRS=\
	easymini-v1.0 easymini-v1.0/flash-loader

AVRDIRS=\
	telescience-v0.1 telescience-pwm micropeak nanopeak-v0.1

SUBDIRS=

ifeq ($(strip $(HAVE_PDCLIB)),yes)
PDCLIB=pdclib
CLEAN_PDCLIB=clean-pdclib
endif

ifeq ($(strip $(HAVE_SDCC)),yes)
SUBDIRS+=$(SDCCDIRS)
endif

ifeq ($(strip $(HAVE_ARM_M3_CC)),yes)
SUBDIRS+=$(ARMM3DIRS)
foo=bar
endif

ifeq ($(strip $(HAVE_ARM_M0_CC)),yes)
SUBDIRS+=$(ARMM0DIRS)
baz=bletch
endif

ifeq ($(strip $(HAVE_AVR_CC)),yes)
SUBDIRS += $(AVRDIRS)
endif

ALLDIRS=$(SDCCDIRS) $(ARMM3DIRS) $(ARMM0DIRS) $(AVRDIRS)

all: all-local all-recursive

RECURSIVE_TARGETS = all-recursive install-recursive

$(RECURSIVE_TARGETS):
	@target=`echo $@ | sed 's/-recursive//'`; \
	for subdir in $(SUBDIRS); do \
		echo "Making $$target in $$subdir"; \
		(cd $$subdir && $(MAKE) $$target) || exit 1; \
	done

ALL_RECURSIVE_TARGETS = clean-recursive

$(ALL_RECURSIVE_TARGETS):
	@target=`echo $@ | sed 's/-recursive//'`; \
	for subdir in $(ALLDIRS); do \
		echo "Making $$target in $$subdir"; \
		(cd $$subdir && $(MAKE) $$target) || exit 1; \
	done

distclean:	clean

clean: clean-local clean-recursive

install: install-recursive

uninstall:

all-recursive: all-local

all-local: altitude.h altitude-pa.h ao_kalman.h ao_whiten.h $(PDCLIB)

altitude.h: make-altitude
	nickle $< > $@

altitude-pa.h: make-altitude-pa
	nickle $< > $@

ao_kalman.h: make-kalman kalman.5c kalman_filter.5c load_csv.5c matrix.5c
	bash $< kalman > $@

ao_whiten.h: make-whiten
	nickle $< > $@

clean-local: $(CLEAN_PDCLIB)
	rm -f altitude.h ao_kalman.h

pdclib:
	cd ../pdclib && mkdir -p opt/include opt/lib && make && make prefix=`pwd`/opt install

clean-pdclib:
	cd ../pdclib && make clean && rm -rf opt
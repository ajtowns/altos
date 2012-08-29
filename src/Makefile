#
# AltOS build
#

vpath make-altitude util
vpath make-altitude-pa util
vpath make-kalman util
vpath make-whiten util
vpath kalman.5c kalman
vpath kalman_filter.5c kalman
vpath load_csv.5c kalman
vpath matrix.5c kalman

include Version

ifneq ($(shell which sdcc),)
	SUBDIRS+=\
	telemetrum-v1.2 telemetrum-v1.1 telemetrum-v1.0 \
	teledongle-v0.2 teledongle-v0.1 \
	telemini-v1.0 telenano-v0.1 \
	telebt-v0.0 telebt-v0.1 \
	telemetrum-v0.1-sky telemetrum-v0.1-sirf \
	telelaunch-v0.1 tidongle test \
	teleterra-v0.2 teleshield-v0.1 \
	telefire-v0.1 \
	spiradio-v0.1
endif

ifneq ($(shell which avr-gcc),)
	SUBDIRS += telescience-v0.1 telepyro-v0.1
endif

ifneq ($(shell which arm-none-eabi-gcc),)
	SUBDIRS += megametrum-v0.1 stm-bringup stm-demo telelco-v0.1
endif

all: all-local all-recursive

RECURSIVE_TARGETS = all-recursive clean-recursive install-recursive

$(RECURSIVE_TARGETS):
	@target=`echo $@ | sed 's/-recursive//'`; \
	for subdir in $(SUBDIRS); do \
		echo "Making $$target in $$subdir"; \
		(cd $$subdir && $(MAKE) $$target) || exit 1; \
	done

distclean:	clean

clean: clean-local clean-recursive

install: install-recursive

uninstall:

all-recursive: all-local

all-local: altitude.h altitude-pa.h ao_kalman.h ao_whiten.h

altitude.h: make-altitude
	nickle $< > $@

altitude-pa.h: make-altitude-pa
	nickle $< > $@

ao_kalman.h: make-kalman kalman.5c kalman_filter.5c load_csv.5c matrix.5c
	bash $< kalman > $@

ao_whiten.h: make-whiten
	nickle $< > $@

clean-local:
	rm -f altitude.h ao_kalman.h

#
# AltOS build
#
#
CC=sdcc

include Version

SUBDIRS=\
	telemetrum-v1.1 telemetrum-v1.0 \
	teledongle-v0.2 teledongle-v0.1 \
	telemini-v0.1 telenano-v0.1 \
	telebt-v0.0 telebt-v0.1 \
	telemetrum-v0.1-sky telemetrum-v0.1-sirf \
	tidongle test

all: all-recursive

RECURSIVE_TARGETS = all-recursive clean-recursive install-recursive

$(RECURSIVE_TARGETS):
	@target=`echo $@ | sed 's/-recursive//'`; \
	for subdir in $(SUBDIRS); do \
		echo "Making $$target in $$subdir"; \
		(cd $$subdir && $(MAKE) $$target) || exit 1; \
	done

distclean:	clean

clean: clean-recursive

install: install-recursive

uninstall:

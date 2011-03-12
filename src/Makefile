#
# AltOS build
#
#
CC=sdcc

include Version

SUBDIRS=\
	telemetrum-v1.1 telemetrum-v1.0 \
	teledongle-v0.2 teledongle-v0.1 \
	telemini-v0.1 \
	telemetrum-v0.1-sky telemetrum-v0.1-sirf \
	tidongle test

all: all-recursive

RECURSIVE_TARGETS = all-recursive clean-recursive install-recursive

all-recursive: $(foreach d,$(SUBDIRS),all-recur-$(d))
clean-recursive: $(foreach d,$(SUBDIRS),clean-recur-$(d))
install-recursive: $(foreach d,$(SUBDIRS),install-recur-$(d))

RECUR_TARGET = $(shell echo "$@" | sed 's/-recur-.*//')
RECUR_SUBDIR = $(shell echo "$@" | sed 's/.*-recur-//')

$(foreach t,all clean install,$(foreach d,$(SUBDIRS),$(t)-recur-$(d))):
	@echo "Making $(RECUR_TARGET) in $(RECUR_SUBDIR)"
	@cd "$(RECUR_SUBDIR)" && $(MAKE) $(RECUR_TARGET)

distclean:	clean

clean: clean-recursive

install: install-recursive

uninstall:

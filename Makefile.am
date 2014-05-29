SUBDIRS=ao-tools src doc icon altoslib libaltos altosuilib altosui micropeak ao-utils altosdroid telegps

EXTRA_DIST = ChangeLog

MAINTAINERCLEANFILES = ChangeLog

.PHONY: ChangeLog

ChangeLog:
	(GIT_DIR=$(top_srcdir)/.git git log > .changelog.tmp && mv .changelog.tmp ChangeLog; rm -f .changelog.tmp) || \
	(touch ChangeLog; echo 'git directory not found: installing possibly empty changelog.' >&2)

dist-hook: ChangeLog

fat:
	cd src && $(MAKE) all
	cd doc && $(MAKE) all
	cd libaltos && $(MAKE) all
	cd altoslib && $(MAKE) all
	cd altosuilib && $(MAKE) all
	cd altosui && $(MAKE) fat
	cd micropeak && $(MAKE) fat
	cd telegps && $(MAKE) fat

set-java-versions:
	$(top_srcdir)/fix-java-versions org.altusmetrum.altoslib=$(ALTOSLIB_VERSION) org.altusmetrum.altosuilib=$(ALTOSUILIB_VERSION)

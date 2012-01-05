SUBDIRS=src doc altoslib altosui ao-tools ao-utils

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
	cd altosui/libaltos && $(MAKE) all
	cd altosui && $(MAKE) fat

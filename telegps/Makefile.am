JAVAROOT=classes
AM_JAVACFLAGS=-target 1.6 -encoding UTF-8 -Xlint:deprecation -Xlint:unchecked -source 6

man_MANS=telegps.1

altoslibdir=$(libdir)/altos

CLASSPATH_ENV=mkdir -p $(JAVAROOT); CLASSPATH=".:classes:../altoslib/*:../altosuilib/*:../libaltos:$(JCOMMON)/jcommon.jar:$(JFREECHART)/jfreechart.jar:$(FREETTS)/freetts.jar"

bin_SCRIPTS=telegps

telegpsdir=$(datadir)/java

telegps_JAVA= \
	TeleGPS.java \
	TeleGPSStatus.java \
	TeleGPSStatusUpdate.java \
	TeleGPSInfo.java \
	TeleGPSState.java \
	TeleGPSConfig.java \
	TeleGPSConfigUI.java \
	TeleGPSPreferences.java \
	TeleGPSGraphUI.java \
	TeleGPSDisplayThread.java

JFREECHART_CLASS= \
    jfreechart.jar

JCOMMON_CLASS=\
    jcommon.jar

FREETTS_CLASS= \
	cmudict04.jar \
	cmulex.jar \
	cmu_time_awb.jar \
	cmutimelex.jar \
	cmu_us_kal.jar \
	en_us.jar \
	freetts.jar

JAR=telegps.jar

FATJAR=telegps-fat.jar

LIBALTOS= \
	libaltos.so \
	libaltos.dylib \
	altos64.dll \
	altos.dll

ALTOSLIB_CLASS=\
	altoslib_$(ALTOSLIB_VERSION).jar

ALTOSUILIB_CLASS=\
	altosuilib_$(ALTOSUILIB_VERSION).jar

# Icons
ICONDIR=$(top_srcdir)/icon

JAVA_ICONS=\
	$(ICONDIR)/telegps-16.png \
	$(ICONDIR)/telegps-32.png \
	$(ICONDIR)/telegps-48.png \
	$(ICONDIR)/telegps-64.png \
	$(ICONDIR)/telegps-128.png \
	$(ICONDIR)/telegps-256.png

# icon base names for jar
ICONJAR= -C $(ICONDIR) telegps-16.png \
	-C $(ICONDIR) telegps-32.png \
	-C $(ICONDIR) telegps-48.png \
	-C $(ICONDIR) telegps-64.png \
	-C $(ICONDIR) telegps-128.png \
	-C $(ICONDIR) telegps-256.png

WINDOWS_ICON=$(ICONDIR)/telegps.ico

# Firmware
FIRMWARE_TD_0_2=$(top_srcdir)/src/teledongle-v0.2/teledongle-v0.2-$(VERSION).ihx
FIRMWARE_TD=$(FIRMWARE_TD_0_2)

FIRMWARE_TBT_1_0=$(top_srcdir)/src/telebt-v1.0/telebt-v1.0-$(VERSION).ihx
FIRMWARE_TBT=$(FIRMWARE_TBT_1_0)

FIRMWARE_TG_1_0=$(top_srcdir)/src/telegps-v1.0/telegps-v1.0-$(VERSION).ihx
FIRMWARE_TG=$(FIRMWARE_TG_1_0)

FIRMWARE=$(FIRMWARE_TG) $(FIRMWARE_TD) $(FIRMWARE_TBT)

desktopdir = $(datadir)/applications
desktop_file = telegps.desktop
desktop_SCRIPTS = $(desktop_file)

all-local: telegps-test telegps-jdb $(JAR)

clean-local:
	-rm -rf classes $(JAR) $(FATJAR) \
		TeleGPS-Linux-*.tar.bz2 TeleGPS-Mac-*.dmg TeleGPS-Windows-*.exe \
		$(ALTOSLIB_CLASS) \
		$(ALTOSUILIB_CLASS) \
		$(JFREECHART_CLASS) $(JCOMMON_CLASS) $(LIBALTOS) Manifest.txt Manifest-fat.txt \
		telegps telegps-test telegps-jdb macosx linux windows telegps-windows.log \
		telegps-windows.nsi

EXTRA_DIST = $(desktop_file).in

$(desktop_file): $(desktop_file).in
	sed -e 's#%bindir%#@bindir@#' -e 's#%icondir%#$(datadir)/icons/hicolor/scalable/apps#' ${srcdir}/telegps.desktop.in > $@
	chmod +x $@

LINUX_DIST=TeleGPS-Linux-$(VERSION).tar.bz2
LINUX_SH=TeleGPS-Linux-$(VERSION).sh
MACOSX_DIST=TeleGPS-Mac-$(VERSION).dmg
WINDOWS_DIST=TeleGPS-Windows-$(VERSION_DASH).exe

TELEGPS_DOC=$(top_srcdir)/doc/telegps.pdf

DOC=$(TELEGPS_DOC)

FAT_FILES=$(FATJAR) $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS) $(FREETTS_CLASS) $(JFREECHART_CLASS) $(JCOMMON_CLASS)

LINUX_FILES=$(FAT_FILES) libaltos.so $(FIRMWARE) $(DOC) telegps.desktop.in ../icon/telegps.svg
LINUX_EXTRA=telegps-fat telegps.desktop.in

MACOSX_INFO_PLIST=Info.plist
MACOSX_README=ReadMe-Mac.rtf
MACOSX_FILES=$(FAT_FILES) libaltos.dylib $(MACOSX_INFO_PLIST) $(MACOSX_README) $(DOC)
MACOSX_EXTRA=$(FIRMWARE)

WINDOWS_FILES=$(FAT_FILES) altos.dll altos64.dll $(top_srcdir)/telemetrum.inf $(DOC) $(WINDOWS_ICON)

if FATINSTALL

FATTARGET=$(FATDIR)/$(VERSION)

LINUX_DIST_TARGET=$(FATTARGET)/$(LINUX_DIST)
LINUX_SH_TARGET=$(FATTARGET)/$(LINUX_SH)
MACOSX_DIST_TARGET=$(FATTARGET)/$(MACOSX_DIST)
WINDOWS_DIST_TARGET=$(FATTARGET)/$(WINDOWS_DIST)

fat: $(LINUX_DIST_TARGET) $(LINUX_SH_TARGET) $(MACOSX_DIST_TARGET) $(WINDOWS_DIST_TARGET)

$(LINUX_DIST_TARGET): $(LINUX_DIST)
	mkdir -p $(FATTARGET)
	cp -p $< $@

$(LINUX_SH_TARGET): $(LINUX_SH)
	mkdir -p $(FATTARGET)
	cp -p $< $@

$(MACOSX_DIST_TARGET): $(MACOSX_DIST)
	mkdir -p $(FATTARGET)
	cp -p $< $@

$(WINDOWS_DIST_TARGET): $(WINDOWS_DIST)
	mkdir -p $(FATTARGET)
	cp -p $< $@

else
fat: $(LINUX_DIST) $(LINUX_SH) $(MACOSX_DIST) $(WINDOWS_DIST)
endif

telegps: Makefile
	echo "#!/bin/sh" > $@
	echo 'exec java  -Djava.library.path="$(altoslibdir)" -jar "$(telegpsdir)/telegps.jar" "$$@"' >> $@
	chmod +x $@

telegps-jdb: Makefile
	echo "#!/bin/sh" > $@
	echo 'exec jdb -classpath "classes:./*:../libaltos:$(JCOMMON)/jcommon.jar:$(JFREECHART)/jfreechart.jar" -Djava.library.path="../libaltos/.libs" org.altusmetrum.telegps.TeleGPS "$$@"' >> $@
	chmod +x $@

telegps-test: Makefile
	echo "#!/bin/sh" > $@
	echo 'exec java -Djava.library.path="../libaltos/.libs" -jar telegps.jar "$$@"' >> $@
	chmod +x $@

install-telegpsJAVA: telegps.jar
	@$(NORMAL_INSTALL)
	test -z "$(telegpsdir)" || $(MKDIR_P) "$(DESTDIR)$(telegpsdir)"
	echo " $(INSTALL_DATA)" "$<" "'$(DESTDIR)$(telegpsdir)/telegps.jar'"; \
	$(INSTALL_DATA) "$<" "$(DESTDIR)$(telegpsdir)"

$(JAR): classtelegps.stamp Manifest.txt $(JAVA_ICONS) $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS)
	jar cfm $@ Manifest.txt \
		$(ICONJAR) \
		-C classes org \
		-C ../libaltos libaltosJNI

$(FATJAR): classtelegps.stamp Manifest-fat.txt $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS) $(JFREECHART_CLASS) $(JCOMMON_CLASS) $(JAVA_ICONS)
	jar cfm $@ Manifest-fat.txt \
		$(ICONJAR) \
		-C classes org \
		-C ../libaltos libaltosJNI

classaltosui.stamp: $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS)

libaltos.so: build-libaltos
	-rm -f "$@"
	$(LN_S) ../libaltos/.libs/"$@" .

libaltos.dylib:
	-rm -f "$@"
	$(LN_S) ../libaltos/"$@" .

altos.dll: ../libaltos/altos.dll
	-rm -f "$@"
	$(LN_S) ../libaltos/"$@" .

altos64.dll: ../libaltos/altos64.dll
	-rm -f "$@"
	$(LN_S) ../libaltos/"$@" .

../libaltos/.libs/libaltos.so: build-libaltos

../libaltos/altos.dll: build-altos-dll

../libaltos/altos64.dll: build-altos64-dll

build-libaltos:
	+cd ../libaltos && make libaltos.la
build-altos-dll:
	+cd ../libaltos && make altos.dll

build-altos64-dll:
	+cd ../libaltos && make altos64.dll

$(ALTOSLIB_CLASS):
	-rm -f "$@"
	$(LN_S) ../altoslib/"$@" .

$(ALTOSUILIB_CLASS):
	-rm -f "$@"
	$(LN_S) ../altosuilib/"$@" .

$(FREETTS_CLASS):
	-rm -f "$@"
	$(LN_S) "$(FREETTS)"/"$@" .

$(JFREECHART_CLASS):
	-rm -f "$@"
	$(LN_S) "$(JFREECHART)"/"$@" .

$(JCOMMON_CLASS):
	-rm -f "$@"
	$(LN_S) "$(JCOMMON)"/"$@" .

$(LINUX_DIST): $(LINUX_FILES) $(LINUX_EXTRA)
	-rm -f $@
	-rm -rf linux
	mkdir -p linux/TeleGPS
	cp -p $(LINUX_FILES) linux/TeleGPS
	cp -p telegps-fat linux/TeleGPS/telegps
	chmod +x linux/TeleGPS/telegps
	tar cjf $@ -C linux TeleGPS

$(LINUX_SH): $(LINUX_DIST) $(srcdir)/../altosui/linux-install.sh
	sed 's/AltOS/TeleGPS/g' $(srcdir)/../altosui/linux-install.sh | cat - $(LINUX_DIST) > $@
	chmod +x $@

$(MACOSX_DIST): $(MACOSX_FILES) $(MACOSX_EXTRA) Makefile
	-rm -f $@
	-rm -rf macosx
	mkdir macosx
	cp -a TeleGPS.app macosx/
	cp -a $(MACOSX_README) macosx/ReadMe.rtf
	cp -a $(DOC) macosx
	cp -p Info.plist macosx/TeleGPS.app/Contents
	mkdir -p macosx/AltOS-$(VERSION) macosx/TeleGPS.app/Contents/Resources/Java
	cp -p $(FATJAR) macosx/TeleGPS.app/Contents/Resources/Java/telegps.jar
	cp -p libaltos.dylib macosx/TeleGPS.app/Contents/Resources/Java
	cp -p $(ALTOSLIB_CLASS) macosx/TeleGPS.app/Contents/Resources/Java
	cp -p $(ALTOSUILIB_CLASS) macosx/TeleGPS.app/Contents/Resources/Java
	cp -p $(FREETTS_CLASS) macosx/TeleGPS.app/Contents/Resources/Java
	cp -p $(JFREECHART_CLASS) macosx/TeleGPS.app/Contents/Resources/Java
	cp -p $(JCOMMON_CLASS) macosx/TeleGPS.app/Contents/Resources/Java
	cp -p $(MACOSX_EXTRA) macosx/AltOS-$(VERSION)
	genisoimage -D -V TeleGPS-$(VERSION) -no-pad -r -apple -o $@ macosx

$(WINDOWS_DIST): $(WINDOWS_FILES) telegps-windows.nsi
	-rm -f $@
	makensis -Otelegps-windows.log "-XOutFile $@" "-DVERSION=$(VERSION)" telegps-windows.nsi || (cat telegps-windows.log && exit 1)

Manifest.txt: Makefile
	echo 'Main-Class: org.altusmetrum.telegps.TeleGPS' > $@
	echo "Class-Path: $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS) $(FREETTS)/freetts.jar $(JCOMMON)/jcommon.jar $(JFREECHART)/jfreechart.jar" >> $@

Manifest-fat.txt:
	echo 'Main-Class: org.altusmetrum.telegps.TeleGPS' > $@
	echo "Class-Path: $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS) freetts.jar jcommon.jar jfreechart.jar" >> $@


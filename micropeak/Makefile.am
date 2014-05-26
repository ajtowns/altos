JAVAROOT=classes
AM_JAVACFLAGS=-target 1.6 -encoding UTF-8 -Xlint:deprecation -Xlint:unchecked -source 6

man_MANS=micropeak.1

altoslibdir=$(libdir)/altos

CLASSPATH_ENV=mkdir -p $(JAVAROOT); CLASSPATH=".:classes:../altoslib/*:../altosuilib/*:../libaltos:$(JCOMMON)/jcommon.jar:$(JFREECHART)/jfreechart.jar"

bin_SCRIPTS=micropeak

micropeakdir=$(datadir)/java

micropeak_JAVA= \
	MicroPeak.java \
	MicroData.java \
	MicroDataPoint.java \
	MicroDownload.java \
	MicroExport.java \
	MicroFile.java \
	MicroFrame.java \
	MicroGraph.java \
	MicroRaw.java \
	MicroSave.java \
	MicroSerial.java \
	MicroSerialLog.java \
	MicroStats.java \
	MicroStatsTable.java \
	MicroFileChooser.java \
	MicroDeviceDialog.java \
	MicroUSB.java

JFREECHART_CLASS= \
    jfreechart.jar

JCOMMON_CLASS=\
    jcommon.jar

JAR=micropeak.jar

FATJAR=micropeak-fat.jar

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
	$(ICONDIR)/micropeak-16.png \
	$(ICONDIR)/micropeak-32.png \
	$(ICONDIR)/micropeak-48.png \
	$(ICONDIR)/micropeak-64.png \
	$(ICONDIR)/micropeak-128.png \
	$(ICONDIR)/micropeak-256.png

# icon base names for jar
ICONJAR= -C $(ICONDIR) micropeak-16.png \
	-C $(ICONDIR) micropeak-32.png \
	-C $(ICONDIR) micropeak-48.png \
	-C $(ICONDIR) micropeak-64.png \
	-C $(ICONDIR) micropeak-128.png \
	-C $(ICONDIR) micropeak-256.png

WINDOWS_ICON=$(ICONDIR)/micro-peak.ico

desktopdir = $(datadir)/applications
desktop_file = micropeak.desktop
desktop_SCRIPTS = $(desktop_file)

all-local: micropeak-test micropeak-jdb $(JAR)

clean-local:
	-rm -rf classes $(JAR) $(FATJAR) \
		MicroPeak-Linux-*.tar.bz2 MicroPeak-Mac-*.dmg MicroPeak-Windows-*.exe \
		$(ALTOSLIB_CLASS) \
		$(ALTOSUILIB_CLASS) \
		$(JFREECHART_CLASS) $(JCOMMON_CLASS) $(LIBALTOS) Manifest.txt Manifest-fat.txt \
		micropeak micropeak-test micropeak-jdb macosx linux windows micropeak-windows.log \
		micropeak-windows.nsi

EXTRA_DIST = $(desktop_file).in

$(desktop_file): $(desktop_file).in
	sed -e 's#%bindir%#@bindir@#' -e 's#%icondir%#$(datadir)/icons/hicolor/scalable/apps#' ${srcdir}/micropeak.desktop.in > $@
	chmod +x $@

LINUX_DIST=MicroPeak-Linux-$(VERSION).tar.bz2
LINUX_SH=MicroPeak-Linux-$(VERSION).sh
MACOSX_DIST=MicroPeak-Mac-$(VERSION).dmg
WINDOWS_DIST=MicroPeak-Windows-$(VERSION_DASH).exe

MICROPEAK_DOC=$(top_srcdir)/doc/micropeak.pdf

DOC=$(MICROPEAK_DOC)

FAT_FILES=$(FATJAR) $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS) $(FREETTS_CLASS) $(JFREECHART_CLASS) $(JCOMMON_CLASS)

LINUX_FILES=$(FAT_FILES) libaltos.so $(FIRMWARE) $(DOC) micropeak.desktop.in ../icon/micropeak.svg
LINUX_EXTRA=micropeak-fat micropeak.desktop.in

MACOSX_DRIVER_URL=http://www.ftdichip.com/Drivers/VCP/MacOSX/FTDIUSBSerialDriver_v2_2_18.dmg
MACOSX_DRIVER=FTDIUSBSerialDriver_v2_2_18.dmg
MACOSX_INFO_PLIST=Info.plist
MACOSX_README=ReadMe-Mac.rtf
MACOSX_FILES=$(FAT_FILES) libaltos.dylib $(MACOSX_INFO_PLIST) $(MACOSX_DRIVER) $(MACOSX_README) $(DOC)

$(MACOSX_DRIVER):
	wget $(MACOSX_DRIVER_URL)

WINDOWS_DRIVER_URL=http://www.ftdichip.com/Drivers/CDM/CDM20824_Setup.exe
WINDOWS_DRIVER=CDM20824_Setup.exe

$(WINDOWS_DRIVER):
	wget $(WINDOWS_DRIVER_URL)

WINDOWS_FILES=$(FAT_FILES) altos.dll altos64.dll $(DOC) $(WINDOWS_ICON) $(WINDOWS_DRIVER)

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

micropeak: Makefile
	echo "#!/bin/sh" > $@
	echo 'exec java  -Djava.library.path="$(altoslibdir)" -jar "$(micropeakdir)/micropeak.jar" "$$@"' >> $@
	chmod +x $@

micropeak-jdb: Makefile
	echo "#!/bin/sh" > $@
	echo 'exec jdb -classpath "classes:./*:../libaltos:$(JCOMMON)/jcommon.jar:$(JFREECHART)/jfreechart.jar" -Djava.library.path="../libaltos/.libs" org.altusmetrum.micropeak.MicroPeak "$$@"' >> $@
	chmod +x $@

micropeak-test: Makefile
	echo "#!/bin/sh" > $@
	echo 'exec java -cp "./*:../libaltos/*:$(JCOMMON)/jcommon.jar:$(JFREECHART)/jfreechart.jar" -Djava.library.path="../libaltos/.libs" -jar micropeak.jar "$$@"' >> $@
	chmod +x $@

install-micropeakJAVA: micropeak.jar
	@$(NORMAL_INSTALL)
	test -z "$(micropeakdir)" || $(MKDIR_P) "$(DESTDIR)$(micropeakdir)"
	echo " $(INSTALL_DATA)" "$<" "'$(DESTDIR)$(micropeakdir)/micropeak.jar'"; \
	$(INSTALL_DATA) "$<" "$(DESTDIR)$(micropeakdir)"

$(JAR): classmicropeak.stamp Manifest.txt $(JAVA_ICONS) $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS)
	jar cfm $@ Manifest.txt \
		$(ICONJAR) \
		-C classes org \
		-C ../libaltos libaltosJNI

$(FATJAR): classmicropeak.stamp Manifest-fat.txt $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS) $(JFREECHART_CLASS) $(JCOMMON_CLASS) $(JAVA_ICONS)
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

$(JFREECHART_CLASS):
	-rm -f "$@"
	$(LN_S) "$(JFREECHART)"/"$@" .

$(JCOMMON_CLASS):
	-rm -f "$@"
	$(LN_S) "$(JCOMMON)"/"$@" .

$(LINUX_DIST): $(LINUX_FILES) $(LINUX_EXTRA)
	-rm -f $@
	-rm -rf linux
	mkdir -p linux/MicroPeak
	cp -p $(LINUX_FILES) linux/MicroPeak
	cp -p micropeak-fat linux/MicroPeak/micropeak
	chmod +x linux/MicroPeak/micropeak
	tar cjf $@ -C linux MicroPeak

$(LINUX_SH): $(LINUX_DIST) $(srcdir)/../altosui/linux-install.sh
	sed 's/AltOS/MicroPeak/g' $(srcdir)/../altosui/linux-install.sh | cat - $(LINUX_DIST) > $@
	chmod +x $@

$(MACOSX_DIST): $(MACOSX_FILES)
	-rm -f $@
	-rm -rf macosx
	mkdir macosx
	cp -a MicroPeak.app macosx/
	cp -a $(MACOSX_README) macosx/ReadMe.rtf
	cp -a $(DOC) macosx
	cp -p Info.plist macosx/MicroPeak.app/Contents
	cp -p $(MACOSX_DRIVER) macosx
	mkdir -p macosx/MicroPeak.app/Contents/Resources/Java
	cp -p $(FATJAR) macosx/MicroPeak.app/Contents/Resources/Java/micropeak.jar
	cp -p libaltos.dylib macosx/MicroPeak.app/Contents/Resources/Java
	cp -p $(ALTOSLIB_CLASS) macosx/MicroPeak.app/Contents/Resources/Java
	cp -p $(ALTOSUILIB_CLASS) macosx/MicroPeak.app/Contents/Resources/Java
	cp -p $(JFREECHART_CLASS) macosx/MicroPeak.app/Contents/Resources/Java
	cp -p $(JCOMMON_CLASS) macosx/MicroPeak.app/Contents/Resources/Java
	genisoimage -D -V MicroPeak-$(VERSION) -no-pad -r -apple -o $@ macosx

$(WINDOWS_DIST): $(WINDOWS_FILES) micropeak-windows.nsi
	-rm -f $@
	makensis -Omicropeak-windows.log "-XOutFile $@" "-DVERSION=$(VERSION)" micropeak-windows.nsi

Manifest.txt: Makefile
	echo 'Main-Class: org.altusmetrum.micropeak.MicroPeak' > $@
	echo "Class-Path: $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS) $(JCOMMON)/jcommon.jar $(JFREECHART)/jfreechart.jar" >> $@

Manifest-fat.txt:
	echo 'Main-Class: org.altusmetrum.micropeak.MicroPeak' > $@
	echo "Class-Path: $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS) jcommon.jar jfreechart.jar" >> $@


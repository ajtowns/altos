JAVAROOT=classes
AM_JAVACFLAGS=-encoding UTF-8 -Xlint:deprecation

CLASSPATH_ENV=mkdir -p $(JAVAROOT); CLASSPATH=".:classes:../altoslib/*:../altosuilib/*:../libaltos:$(JCOMMON)/jcommon.jar:$(JFREECHART)/jfreechart.jar"

bin_SCRIPTS=micropeak

micropeakdir=$(datadir)/java

micropeak_JAVA= \
	MicroPeak.java \
	MicroData.java \
	MicroDownload.java \
	MicroFrame.java \
	MicroGraph.java \
	MicroSave.java \
	MicroSerial.java \
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
	AltosLib.jar

ALTOSUILIB_CLASS=\
	AltosUILib.jar

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

all-local: micropeak-test $(JAR)

clean-local:
	-rm -rf classes $(JAR) $(FATJAR) \
		$(LINUX_DIST) $(MACOSX_DIST) windows $(WINDOWS_DIST) \
		$(ALTOSLIB_CLASS) \
		$(ALTOSUILIB_CLASS) \
		$(JFREECHART_CLASS) $(JCOMMON_CLASS) $(LIBALTOS) Manifest.txt \
		micropeak micropeak-test macosx linux windows

LINUX_DIST=MicroPeak-Linux-$(VERSION).tar.bz2
MACOSX_DIST=MicroPeak-Mac-$(VERSION).zip
WINDOWS_DIST=MicroPeak-Windows-$(VERSION_DASH).exe

FAT_FILES=$(FATJAR) $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS) $(FREETTS_CLASS) $(JFREECHART_CLASS) $(JCOMMON_CLASS)

LINUX_FILES=$(FAT_FILES) libaltos.so $(FIRMWARE) $(DOC)
LINUX_EXTRA=micropeak-fat

MACOSX_INFO_PLIST=Info.plist
MACOSX_FILES=$(FAT_FILES) libaltos.dylib $(MACOSX_INFO_PLIST)

WINDOWS_FILES=$(FAT_FILES) altos.dll altos64.dll $(top_srcdir)/telemetrum.inf $(WINDOWS_ICON)

if FATINSTALL

FATTARGET=$(FATDIR)/$(VERSION)

LINUX_DIST_TARGET=$(FATTARGET)/$(LINUX_DIST)
MACOSX_DIST_TARGET=$(FATTARGET)/$(MACOSX_DIST)
WINDOWS_DIST_TARGET=$(FATTARGET)/$(WINDOWS_DIST)

fat: $(LINUX_DIST_TARGET) $(MACOSX_DIST_TARGET) $(WINDOWS_DIST_TARGET)

$(LINUX_DIST_TARGET): $(LINUX_DIST)
	mkdir -p $(FATTARGET)
	cp -p $< $@

$(MACOSX_DIST_TARGET): $(MACOSX_DIST)
	mkdir -p $(FATTARGET)
	cp -p $< $@

$(WINDOWS_DIST_TARGET): $(WINDOWS_DIST)
	mkdir -p $(FATTARGET)
	cp -p $< $@

else
fat: $(LINUX_DIST) $(MACOSX_DIST) $(WINDOWS_DIST)
endif

micropeak: Makefile
	echo "#!/bin/sh" > $@
	echo 'exec java  -cp "$(JCOMMON)/jcommon.jar:$(JFREECHART)/jfreechart.jar" -Djava.library.path="$(altoslibdir)" -jar "$(micropeakdir)/micropeak.jar" "$$@"' >> $@
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

$(MACOSX_DIST): $(MACOSX_FILES)
	-rm -f $@
	-rm -rf macosx
	mkdir macosx
	cp -a MicroPeak.app macosx/
	cp -p Info.plist macosx/MicroPeak.app/Contents
	mkdir -p macosx/MicroPeak macosx/MicroPeak.app/Contents/Resources/Java
	cp -p $(FATJAR) macosx/MicroPeak.app/Contents/Resources/Java/micropeak.jar
	cp -p libaltos.dylib macosx/MicroPeak.app/Contents/Resources/Java
	cp -p $(ALTOSLIB_CLASS) macosx/MicroPeak.app/Contents/Resources/Java
	cp -p $(ALTOSUILIB_CLASS) macosx/MicroPeak.app/Contents/Resources/Java
	cp -p $(JFREECHART_CLASS) macosx/MicroPeak.app/Contents/Resources/Java
	cp -p $(JCOMMON_CLASS) macosx/MicroPeak.app/Contents/Resources/Java
	cd macosx && zip -r ../$@ MicroPeak.app MicroPeak

$(WINDOWS_DIST): $(WINDOWS_FILES) micropeak-windows.nsi
	-rm -f $@
	makensis -Omicropeak-windows.log "-XOutFile $@" "-DVERSION=$(VERSION)" micropeak-windows.nsi

Manifest.txt: Makefile
	echo 'Main-Class: org.altusmetrum.micropeak.MicroPeak' > $@
	echo "Class-Path: AltosLib.jar AltosUILib.jar $(JCOMMON)/jcommon.jar $(JFREECHART)/jfreechart.jar" >> $@

Manifest-fat.txt:
	echo 'Main-Class: org.altusmetrum.micropeak.MicroPeak' > $@
	echo "Class-Path: AltosLib.jar AltosUILib.jar jcommon.jar jfreechart.jar" >> $@


JAVAROOT=classes
AM_JAVACFLAGS=-encoding UTF-8

altoslibdir=$(libdir)/altos

CLASSPATH_ENV=mkdir -p $(JAVAROOT); CLASSPATH=".:classes:../libaltos:$(FREETTS)/*:/usr/share/java/*"

bin_SCRIPTS=altosui

altosui_JAVA = \
	AltosChannelMenu.java \
	AltosConfig.java \
	AltosConfigUI.java \
	AltosConvert.java \
	AltosCRCException.java \
	AltosCSV.java \
	AltosCSVUI.java \
	AltosDebug.java \
	AltosDeviceDialog.java \
	AltosDevice.java \
	AltosEepromDownload.java \
	AltosEepromMonitor.java \
	AltosEepromReader.java \
	AltosEepromRecord.java \
	AltosFile.java \
	AltosFlash.java \
	AltosFlashUI.java \
	AltosFlightInfoTableModel.java \
	AltosFlightStatusTableModel.java \
	AltosGPS.java \
	AltosGreatCircle.java \
	AltosHexfile.java \
	Altos.java \
	AltosLine.java \
	AltosLogfileChooser.java \
	AltosLog.java \
	AltosParse.java \
	AltosPreferences.java \
	AltosReader.java \
	AltosRecord.java \
	AltosRomconfig.java \
	AltosRomconfigUI.java \
	AltosSerial.java \
	AltosSerialMonitor.java \
	AltosState.java \
	AltosTelemetry.java \
	AltosTelemetryReader.java \
	AltosUI.java \
	AltosVoice.java

FREETTS_CLASS= \
	cmudict04.jar \
	cmulex.jar \
	cmu_time_awb.jar \
	cmutimelex.jar \
	cmu_us_kal.jar \
	en_us.jar \
	freetts.jar

LIBALTOS= \
	libaltos.so \
	libaltos.dylib \
	altos.dll

JAR=altosui.jar

FATJAR=altosui-fat.jar

# Icons
JAVA_ICON=$(top_srcdir)/icon/altus-metrum-16x16.jpg
WINDOWS_ICON=$(top_srcdir)/icon/altus-metrum.ico

# Firmware
FIRMWARE_TD=$(top_srcdir)/src/teledongle-v0.2-$(VERSION).ihx
FIRMWARE_TM=$(top_srcdir)/src/telemetrum-v1.0-$(VERSION).ihx
FIRMWARE=$(FIRMWARE_TM) $(FIRMWARE_TD)

# Distribution targets
LINUX_DIST=Altos-Linux-$(VERSION).tar.bz2
MACOSX_DIST=Altos-Mac-$(VERSION).zip
WINDOWS_DIST=Altos-Windows-$(VERSION_DASH).exe

FAT_FILES=$(FATJAR) $(FREETTS_CLASS)

LINUX_FILES=$(FAT_FILES) libaltos.so $(FIRMWARE)
LINUX_EXTRA=altosui-fat

MACOSX_FILES=$(FAT_FILES) libaltos.dylib
MACOSX_EXTRA=$(FIRMWARE)

WINDOWS_FILES=$(FAT_FILES) altos.dll $(top_srcdir)/telemetrum.inf $(WINDOWS_ICON)

all-local: classes/altosui $(JAR) altosui altosui-test

clean-local:
	-rm -rf classes $(JAR) $(FATJAR) \
		$(LINUX_DIST) $(MACOSX_DIST) windows $(WINDOWS_DIST) $(FREETTS_CLASS) \
		$(LIBALTOS) Manifest.txt Manifest-fat.txt altos-windows.log \
		altosui altosui-test macosx linux

fat: $(FATJAR) $(LINUX_DIST) $(MACOSX_DIST) $(WINDOWS_DIST)

altosuidir=$(datadir)/java

install-altosuiJAVA: altosui.jar
	@$(NORMAL_INSTALL)
	test -z "$(altosuidir)" || $(MKDIR_P) "$(DESTDIR)$(altosuidir)"
	echo " $(INSTALL_DATA)" "$<" "'$(DESTDIR)$(altosuidir)/altosui.jar'"; \
	$(INSTALL_DATA) "$<" "$(DESTDIR)$(altosuidir)"

classes/altosui:
	mkdir -p classes/altosui

$(JAR): classaltosui.stamp Manifest.txt $(JAVA_ICON)
	jar cfm $@ Manifest.txt \
		-C $(top_srcdir)/icon altus-metrum-16x16.jpg \
		-C classes altosui \
		-C ../libaltos libaltosJNI

$(FATJAR): classaltosui.stamp Manifest-fat.txt $(FREETTS_CLASS) $(LIBALTOS) $(JAVA_ICON)
	jar cfm $@ Manifest-fat.txt \
		-C $(top_srcdir)/icon altus-metrum-16x16.jpg \
		-C classes altosui \
		-C ../libaltos libaltosJNI

Manifest.txt: Makefile
	echo 'Main-Class: altosui.AltosUI' > $@
	echo "Class-Path: $(FREETTS)/freetts.jar" >> $@

Manifest-fat.txt:
	echo 'Main-Class: altosui.AltosUI' > $@
	echo "Class-Path: freetts.jar" >> $@

altosui: Makefile
	echo "#!/bin/sh" > $@
	echo 'exec java  -cp "$(FREETTS)/*" -Djava.library.path="$(altoslibdir)" -jar "$(altosuidir)/altosui.jar" "$$@"' >> $@
	chmod +x $@

altosui-test: Makefile
	echo "#!/bin/sh" > $@
	echo 'exec java -cp "$(FREETTS)/*" -Djava.library.path="../libaltos/.libs" -jar altosui.jar "$$@"' >> $@
	chmod +x $@

$(LIBALTOS):
	-rm -f "$@"
	$(LN_S) ../libaltos/"$@" .

$(FREETTS_CLASS):
	-rm -f "$@"
	$(LN_S) "$(FREETTS)"/"$@" .

$(LINUX_DIST): $(LINUX_FILES) $(LINUX_EXTRA)
	-rm -f $@
	-rm -rf linux
	mkdir -p linux/AltOS
	cp -p $(LINUX_FILES) linux/AltOS
	cp -p altosui-fat linux/AltOS/altosui
	chmod +x linux/AltOS/altosui
	tar cjf $@ -C linux AltOS

$(MACOSX_DIST): $(MACOSX_FILES) $(MACOSX_EXTRA)
	-rm -f $@
	-rm -rf macosx
	mkdir macosx
	cp -a AltosUI.app macosx/
	mkdir -p macosx/AltOS macosx/AltosUI.app/Contents/Resources/Java
	cp -p $(FATJAR) macosx/AltosUI.app/Contents/Resources/Java/altosui.jar
	cp -p $(FREETTS_CLASS) libaltos.dylib macosx/AltosUI.app/Contents/Resources/Java
	cp -p $(MACOSX_EXTRA) macosx/AltOS
	cd macosx && zip -r ../$@ AltosUI.app AltOS

$(WINDOWS_DIST): $(WINDOWS_FILES) altos-windows.nsi
	-rm -f $@
	makensis -Oaltos-windows.log "-XOutFile $@" "-DVERSION=$(VERSION)" altos-windows.nsi
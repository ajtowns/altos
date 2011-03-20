SUBDIRS=libaltos
JAVAROOT=classes
AM_JAVACFLAGS=-encoding UTF-8

man_MANS=altosui.1

altoslibdir=$(libdir)/altos

CLASSPATH_ENV=mkdir -p $(JAVAROOT); CLASSPATH=".:classes:libaltos:$(FREETTS)/*:/usr/share/java/*"

bin_SCRIPTS=altosui

altosui_JAVA = \
	GrabNDrag.java \
	AltosAscent.java \
	AltosChannelMenu.java \
	AltosConfig.java \
	AltosConfigData.java \
	AltosConfigUI.java \
	AltosConfigureUI.java \
	AltosConvert.java \
	AltosCRCException.java \
	AltosCSV.java \
	AltosCSVUI.java \
	AltosDebug.java \
	AltosDescent.java \
	AltosDeviceDialog.java \
	AltosDevice.java \
	AltosDisplayThread.java \
	AltosEepromBlock.java \
	AltosEepromDelete.java \
	AltosEepromDownload.java \
	AltosEepromList.java \
	AltosEepromLog.java \
	AltosEepromManage.java \
	AltosEepromMonitor.java \
	AltosEepromIterable.java \
	AltosEepromRecord.java \
	AltosEepromSelect.java \
	AltosFile.java \
	AltosFlash.java \
	AltosFlashUI.java \
	AltosFlightDisplay.java \
	AltosFlightInfoTableModel.java \
	AltosFlightReader.java \
	AltosFlightStatus.java \
	AltosFlightUI.java \
	AltosGPS.java \
	AltosGreatCircle.java \
	AltosHexfile.java \
	Altos.java \
	AltosIgnite.java \
	AltosIgniteUI.java \
	AltosInfoTable.java \
	AltosKML.java \
	AltosLanded.java \
	AltosLed.java \
	AltosLights.java \
	AltosLine.java \
	AltosLog.java \
	AltosPad.java \
	AltosParse.java \
	AltosPreferences.java \
	AltosReader.java \
	AltosRecord.java \
	AltosRecordIterable.java \
	AltosTelemetryReader.java \
	AltosReplayReader.java \
	AltosRomconfig.java \
	AltosRomconfigUI.java \
	AltosSerial.java \
	AltosSerialInUseException.java \
	AltosSerialMonitor.java \
	AltosSiteMap.java \
	AltosSiteMapCache.java \
	AltosSiteMapTile.java \
	AltosState.java \
	AltosTelemetry.java \
	AltosTelemetryIterable.java \
	AltosUI.java \
	AltosWriter.java \
	AltosDataPointReader.java \
	AltosDataPoint.java \
	AltosGraph.java \
	AltosGraphTime.java \
	AltosGraphUI.java \
	AltosDataChooser.java \
	AltosVersion.java \
	AltosVoice.java

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

LIBALTOS= \
	libaltos.so \
	libaltos.dylib \
	altos.dll

JAR=altosui.jar

FATJAR=altosui-fat.jar

# Icons
ICONDIR=$(top_srcdir)/icon

JAVA_ICON=$(ICONDIR)/altus-metrum-16x16.jpg

ICONS=	$(ICONDIR)/redled.png $(ICONDIR)/redoff.png \
	$(ICONDIR)/greenled.png $(ICONDIR)/greenoff.png \
	$(ICONDIR)/grayled.png $(ICONDIR)/grayoff.png

# icon base names for jar
ICONJAR= -C $(ICONDIR) altus-metrum-16x16.jpg \
	-C $(ICONDIR) redled.png -C $(ICONDIR) redoff.png \
	-C $(ICONDIR) greenled.png -C $(ICONDIR) greenoff.png \
	-C $(ICONDIR) grayon.png -C $(ICONDIR) grayled.png

WINDOWS_ICON=$(ICONDIR)/altus-metrum.ico

# Firmware
FIRMWARE_TD_0_2=$(top_srcdir)/src/teledongle-v0.2-$(VERSION).ihx
FIRMWARE_TD=$(FIRMWARE_TD_0_2)

FIRMWARE_TM_1_0=$(top_srcdir)/src/telemetrum-v1.0-$(VERSION).ihx
FIRMWARE_TM_1_1=$(top_srcdir)/src/telemetrum-v1.1-$(VERSION).ihx
FIRMWARE_TM=$(FIRMWARE_TM_1_0) $(FIRMWARE_TM_1_1)

FIRMWARE=$(FIRMWARE_TM) $(FIRMWARE_TD)

ALTUSMETRUM_DOC=$(top_srcdir)/doc/altusmetrum.pdf
ALTOS_DOC=$(top_srcdir)/doc/altos.pdf
TEMPLATE_DOC=$(top_srcdir)/doc/telemetrum-outline.pdf

DOC=$(ALTUSMETRUM_DOC) $(ALTOS_DOC) $(TEMPLATE_DOC)

# Distribution targets
LINUX_DIST=Altos-Linux-$(VERSION).tar.bz2
MACOSX_DIST=Altos-Mac-$(VERSION).zip
WINDOWS_DIST=Altos-Windows-$(VERSION_DASH).exe

FAT_FILES=$(FATJAR) $(FREETTS_CLASS) $(JFREECHART_CLASS) $(JCOMMON_CLASS)

LINUX_FILES=$(FAT_FILES) libaltos.so $(FIRMWARE) $(DOC)
LINUX_EXTRA=altosui-fat

MACOSX_FILES=$(FAT_FILES) libaltos.dylib
MACOSX_EXTRA=$(FIRMWARE)

WINDOWS_FILES=$(FAT_FILES) altos.dll $(top_srcdir)/telemetrum.inf $(WINDOWS_ICON)

all-local: classes/altosui $(JAR) altosui altosui-test altosui-jdb

clean-local:
	-rm -rf classes $(JAR) $(FATJAR) \
		$(LINUX_DIST) $(MACOSX_DIST) windows $(WINDOWS_DIST) $(FREETTS_CLASS) \
		$(JFREECHART_CLASS) $(JCOMMON_CLASS) $(LIBALTOS) Manifest.txt Manifest-fat.txt altos-windows.log \
		altosui altosui-test altosui-jdb macosx linux

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
		$(ICONJAR) \
		-C classes altosui \
		-C libaltos libaltosJNI

$(FATJAR): classaltosui.stamp Manifest-fat.txt $(FREETTS_CLASS) $(JFREECHART_CLASS) $(JCOMMON_CLASS) $(LIBALTOS) $(JAVA_ICON)
	jar cfm $@ Manifest-fat.txt \
		$(ICONJAR) \
		-C classes altosui \
		-C libaltos libaltosJNI

Manifest.txt: Makefile
	echo 'Main-Class: altosui.AltosUI' > $@
	echo "Class-Path: $(FREETTS)/freetts.jar $(JFREECHART)/jfreechart.jar $(JCOMMON)/jcommon.jar" >> $@

Manifest-fat.txt:
	echo 'Main-Class: altosui.AltosUI' > $@
	echo "Class-Path: freetts.jar jfreechart.jar jcommon.jar" >> $@

altosui: Makefile
	echo "#!/bin/sh" > $@
	echo 'exec java  -cp "$(FREETTS)/*:$(JFREECHART)/*:$(JCOMMON)/*" -Djava.library.path="$(altoslibdir)" -jar "$(altosuidir)/altosui.jar" "$$@"' >> $@
	chmod +x $@

altosui-test: Makefile
	echo "#!/bin/sh" > $@
	echo 'exec java -cp "$(FREETTS)/*:$(JFREECHART)/*:$(JCOMMON)/*" -Djava.library.path="libaltos/.libs" -jar altosui.jar "$$@"' >> $@
	chmod +x $@

altosui-jdb: Makefile
	echo "#!/bin/sh" > $@
	echo 'exec jdb -classpath "classes:libaltos:$(FREETTS)/*:$(JFREECHART)/*:$(JCOMMON)/*" -Djava.library.path="libaltos/.libs" altosui/AltosUI "$$@"' >> $@
	chmod +x $@

libaltos.so:
	-rm -f "$@"
	$(LN_S) libaltos/.libs/"$@" .

libaltos.dylib:
	-rm -f "$@"
	$(LN_S) libaltos/"$@" .

altos.dll:
	-rm -f "$@"
	$(LN_S) libaltos/"$@" .

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
	cp -p $(JFREECHART_CLASS) libaltos.dylib macosx/AltosUI.app/Contents/Resources/Java
	cp -p $(JCOMMON_CLASS) libaltos.dylib macosx/AltosUI.app/Contents/Resources/Java
	cp -p $(MACOSX_EXTRA) macosx/AltOS
	cd macosx && zip -r ../$@ AltosUI.app AltOS

$(WINDOWS_DIST): $(WINDOWS_FILES) altos-windows.nsi
	-rm -f $@
	makensis -Oaltos-windows.log "-XOutFile $@" "-DVERSION=$(VERSION)" altos-windows.nsi


JAVAROOT=classes
AM_JAVACFLAGS=-target 1.6 -encoding UTF-8 -Xlint:deprecation -Xlint:unchecked -source 6

man_MANS=altosui.1

altoslibdir=$(libdir)/altos

CLASSPATH_ENV=mkdir -p $(JAVAROOT); CLASSPATH="$(JAVAROOT):../altoslib/*:../altosuilib/*:../libaltos:$(JCOMMON)/jcommon.jar:$(JFREECHART)/jfreechart.jar:$(FREETTS)/freetts.jar"

bin_SCRIPTS=altosui

altosui_JAVA = \
	AltosAscent.java \
	AltosChannelMenu.java \
	AltosCompanionInfo.java \
	AltosConfig.java \
	AltosConfigUI.java \
	AltosConfigPyroUI.java \
	AltosConfigureUI.java \
	AltosConfigTD.java \
	AltosConfigTDUI.java \
	AltosDescent.java \
	AltosFlightStatus.java \
	AltosFlightStatusUpdate.java \
	AltosFlightUI.java \
	Altos.java \
	AltosIdleMonitorUI.java \
	AltosIgniteUI.java \
	AltosIgnitor.java \
	AltosLaunch.java \
	AltosLaunchUI.java \
	AltosLanded.java \
	AltosPad.java \
	AltosUIPreferencesBackend.java \
	AltosUI.java \
	AltosGraphUI.java

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

ALTOSLIB_CLASS=\
	altoslib_$(ALTOSLIB_VERSION).jar

ALTOSUILIB_CLASS=\
	altosuilib_$(ALTOSUILIB_VERSION).jar

LIBALTOS= \
	libaltos32.so \
	libaltos64.so \
	libaltos.dylib \
	altos.dll

desktopdir = $(datadir)/applications
desktop_file = altos.desktop
desktop_SCRIPTS = $(desktop_file)

JAR=altosui.jar

FATJAR=altosui-fat.jar

# Icons
ICONDIR=$(top_srcdir)/icon

JAVA_ICONS=\
	$(ICONDIR)/altus-metrum-16.png \
	$(ICONDIR)/altus-metrum-32.png \
	$(ICONDIR)/altus-metrum-48.png \
	$(ICONDIR)/altus-metrum-64.png \
	$(ICONDIR)/altus-metrum-128.png \
	$(ICONDIR)/altus-metrum-256.png

# icon base names for jar
ICONJAR= -C $(ICONDIR) altus-metrum-16.png \
	-C $(ICONDIR) altus-metrum-32.png \
	-C $(ICONDIR) altus-metrum-48.png \
	-C $(ICONDIR) altus-metrum-64.png \
	-C $(ICONDIR) altus-metrum-128.png \
	-C $(ICONDIR) altus-metrum-256.png

WINDOWS_ICON=$(ICONDIR)/altus-metrum.ico

# Firmware
FIRMWARE_TD_0_2=$(top_srcdir)/src/teledongle-v0.2/teledongle-v0.2-$(VERSION).ihx
FIRMWARE_TD=$(FIRMWARE_TD_0_2)

FIRMWARE_TM_1_0=$(top_srcdir)/src/telemetrum-v1.0/telemetrum-v1.0-$(VERSION).ihx
FIRMWARE_TM_1_1=$(top_srcdir)/src/telemetrum-v1.1/telemetrum-v1.1-$(VERSION).ihx
FIRMWARE_TM_1_2=$(top_srcdir)/src/telemetrum-v1.2/telemetrum-v1.2-$(VERSION).ihx
FIRMWARE_TM=$(FIRMWARE_TM_1_0) $(FIRMWARE_TM_1_1) $(FIRMWARE_TM_1_2)

FIRMWARE_TELEMINI_1_0=$(top_srcdir)/src/telemini-v1.0/telemini-v1.0-$(VERSION).ihx
FIRMWARE_TELEMINI=$(FIRMWARE_TELEMINI_1_0)

FIRMWARE_TBT_1_0=$(top_srcdir)/src/telebt-v1.0/telebt-v1.0-$(VERSION).ihx
FIRMWARE_TBT=$(FIRMWARE_TBT_1_0)

FIRMWARE_TMEGA_1_0=$(top_srcdir)/src/telemega-v1.0/telemega-v1.0-$(VERSION).ihx
FIRMWARE_TMEGA=$(FIRMWARE_TMEGA_1_0)

FIRMWARE_EMINI_1_0=$(top_srcdir)/src/easymini-v1.0/easymini-v1.0-$(VERSION).ihx
FIRMWARE_EMINI=$(FIRMWARE_EMINI_1_0)

FIRMWARE=$(FIRMWARE_TM) $(FIRMWARE_TELEMINI) $(FIRMWARE_TD) $(FIRMWARE_TBT) $(FIRMWARE_TMEGA) $(FIRMWARE_EMINI)

ALTUSMETRUM_DOC=$(top_srcdir)/doc/altusmetrum.pdf
ALTOS_DOC=$(top_srcdir)/doc/altos.pdf
TELEMETRY_DOC=$(top_srcdir)/doc/telemetry.pdf
TEMPLATE_DOC=\
	$(top_srcdir)/doc/telemetrum-outline.pdf \
	$(top_srcdir)/doc/easymini-outline.pdf \
	$(top_srcdir)/doc/telemega-outline.pdf \
	$(top_srcdir)/doc/telemini.pdf

DOC=$(ALTUSMETRUM_DOC) $(ALTOS_DOC) $(TELEMETRY_DOC) $(TEMPLATE_DOC)

# Distribution targets
LINUX_DIST=Altos-Linux-$(VERSION).tar.bz2
LINUX_SH=Altos-Linux-$(VERSION).sh
MACOSX_DIST=Altos-Mac-$(VERSION).dmg
WINDOWS_DIST=Altos-Windows-$(VERSION_DASH).exe

FAT_FILES=$(FATJAR) $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS) $(FREETTS_CLASS) $(JFREECHART_CLASS) $(JCOMMON_CLASS)

LINUX_LIBS=libaltos32.so libaltos64.so

LINUX_FILES=$(FAT_FILES) $(LINUX_LIBS) $(FIRMWARE) $(DOC) altos.desktop.in ../icon/altusmetrum.svg
LINUX_EXTRA=altosui-fat

MACOSX_INFO_PLIST=Info.plist
MACOSX_FILES=$(FAT_FILES) libaltos.dylib $(MACOSX_INFO_PLIST) $(DOC) ReadMe-Mac.rtf
MACOSX_EXTRA=$(FIRMWARE)

WINDOWS_FILES=$(FAT_FILES) altos.dll altos64.dll $(top_srcdir)/telemetrum.inf $(WINDOWS_ICON)

all-local: classes/altosui $(JAR) altosui altosui-test altosui-jdb

clean-local:
	-rm -rf classes $(JAR) $(FATJAR) \
		$(LINUX_DIST) $(LINUX_SH) $(MACOSX_DIST) windows $(WINDOWS_DIST) $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS) $(FREETTS_CLASS) \
		$(JFREECHART_CLASS) $(JCOMMON_CLASS) $(LIBALTOS) Manifest.txt Manifest-fat.txt altos-windows.log altos-windows.nsi \
		altosui altosui-test altosui-jdb macosx linux

EXTRA_DIST = $(desktop_file).in

$(desktop_file): $(desktop_file).in
	sed -e 's#%bindir%#@bindir@#' -e 's#%icondir%#$(datadir)/icons/hicolor/scalable/apps#' ${srcdir}/altos.desktop.in > $@
	chmod +x $@

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


altosuidir=$(datadir)/java

install-altosuiJAVA: altosui.jar
	@$(NORMAL_INSTALL)
	test -z "$(altosuidir)" || $(MKDIR_P) "$(DESTDIR)$(altosuidir)"
	echo " $(INSTALL_DATA)" "$<" "'$(DESTDIR)$(altosuidir)/altosui.jar'"; \
	$(INSTALL_DATA) "$<" "$(DESTDIR)$(altosuidir)"

classes/altosui:
	mkdir -p classes/altosui

$(JAR): classaltosui.stamp Manifest.txt $(JAVA_ICONS) $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS)
	jar cfm $@ Manifest.txt \
		$(ICONJAR) \
		-C classes altosui \
		-C ../libaltos libaltosJNI

$(FATJAR): classaltosui.stamp Manifest-fat.txt $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS) $(FREETTS_CLASS) $(JFREECHART_CLASS) $(JCOMMON_CLASS) $(LIBALTOS) $(JAVA_ICONS)
	jar cfm $@ Manifest-fat.txt \
		$(ICONJAR) \
		-C classes altosui \
		-C ../libaltos libaltosJNI

Manifest.txt: Makefile
	echo 'Main-Class: altosui.AltosUI' > $@
	echo "Class-Path: $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS) $(FREETTS)/freetts.jar $(JCOMMON)/jcommon.jar $(JFREECHART)/jfreechart.jar" >> $@

Manifest-fat.txt:
	echo 'Main-Class: altosui.AltosUI' > $@
	echo "Class-Path: $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS) freetts.jar jcommon.jar jfreechart.jar" >> $@

altosui: Makefile
	echo "#!/bin/sh" > $@
	echo 'exec java -Djava.library.path="$(altoslibdir)" -jar "$(altosuidir)/altosui.jar" "$$@"' >> $@
	chmod +x $@

altosui-test: Makefile
	echo '#!/bin/sh' > $@
	echo 'dir="$$(dirname $$0)"' >> $@
	echo 'cd "$$dir"' >> $@
	echo 'altosui="$$(pwd -P)"' >> $@
	echo 'altos="$$(dirname $$altosui)"' >> $@
	echo 'exec java -Djava.library.path="$$altos/libaltos/.libs" -jar "$$altosui/altosui.jar" "$$@"' >> $@
	chmod +x $@

altosui-jdb: Makefile
	echo "#!/bin/sh" > $@
	echo 'exec jdb -classpath "classes:./*:../libaltos:$(FREETTS)/freetts.jar:$(JCOMMON)/jcommon.jar:$(JFREECHART)/jfreechart.jar" -Djava.library.path="../libaltos/.libs" altosui/AltosUI "$$@"' >> $@
	chmod +x $@

libaltos32.so: build-libaltos
	-rm -f "$@"
	$(LN_S) ../libaltos/.libs/"$@" .

libaltos64.so: build-libaltos
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

../libaltos/.libs/libaltos64.so: ../libaltos/.libs/libaltos32.so

../libaltos/.libs/libaltos32.so: build-libaltos

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
	mkdir -p linux/AltOS
	cp -p $(LINUX_FILES) linux/AltOS
	cp -p altosui-fat linux/AltOS/altosui
	chmod +x linux/AltOS/altosui
	tar cjf $@ -C linux AltOS

$(LINUX_SH): $(LINUX_DIST) linux-install.sh
	cat linux-install.sh $(LINUX_DIST) > $@
	chmod +x $@

$(MACOSX_DIST): $(MACOSX_FILES) $(MACOSX_EXTRA) Makefile
	-rm -f $@
	-rm -rf macosx
	mkdir macosx
	cp -a AltosUI.app macosx/
	cp -a ReadMe-Mac.rtf macosx/ReadMe.rtf
	mkdir -p macosx/Doc
	cp -a $(DOC) macosx/Doc
	cp -p Info.plist macosx/AltosUI.app/Contents
	mkdir -p macosx/AltOS-$(VERSION) macosx/AltosUI.app/Contents/Resources/Java
	cp -p $(FATJAR) macosx/AltosUI.app/Contents/Resources/Java/altosui.jar
	cp -p libaltos.dylib macosx/AltosUI.app/Contents/Resources/Java
	cp -p $(ALTOSLIB_CLASS) macosx/AltosUI.app/Contents/Resources/Java
	cp -p $(ALTOSUILIB_CLASS) macosx/AltosUI.app/Contents/Resources/Java
	cp -p $(FREETTS_CLASS) macosx/AltosUI.app/Contents/Resources/Java
	cp -p $(JFREECHART_CLASS) macosx/AltosUI.app/Contents/Resources/Java
	cp -p $(JCOMMON_CLASS) macosx/AltosUI.app/Contents/Resources/Java
	cp -p $(MACOSX_EXTRA) macosx/AltOS-$(VERSION)
	genisoimage -D -V AltOS-$(VERSION) -no-pad -r -apple -o $@ macosx

$(WINDOWS_DIST): $(WINDOWS_FILES) altos-windows.nsi
	-rm -f $@
	makensis -Oaltos-windows.log "-XOutFile $@" "-DVERSION=$(VERSION)" altos-windows.nsi

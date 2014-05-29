AM_JAVACFLAGS=-target 1.6 -encoding UTF-8 -Xlint:deprecation -Xlint:unchecked -source 6

JAVAROOT=bin

CLASSPATH_ENV=mkdir -p $(JAVAROOT); CLASSPATH="$(JAVAROOT):../altoslib/*:../libaltos:/usr/share/java/*"

SRC=.

altosuilibdir = $(datadir)/java

altosuilib_JAVA = \
	GrabNDrag.java \
	AltosDevice.java \
	AltosDeviceDialog.java \
	AltosFlightDisplay.java \
	AltosFontListener.java \
	AltosPositionListener.java \
	AltosUIConfigure.java \
	AltosUIAxis.java \
	AltosUIDataMissing.java \
	AltosUIDataPoint.java \
	AltosUIDataSet.java \
	AltosUIGraph.java \
	AltosUIGrapher.java \
	AltosUIDialog.java \
	AltosUIEnable.java \
	AltosUIFrame.java \
	AltosUILib.java \
	AltosUIListener.java \
	AltosUIMarker.java \
	AltosUIPreferencesBackend.java \
	AltosUIPreferences.java \
	AltosUISeries.java \
	AltosUIVersion.java \
	AltosUSBDevice.java \
	AltosSiteMap.java \
	AltosSiteMapCache.java \
	AltosSiteMapPreload.java \
	AltosSiteMapTile.java \
	AltosSiteMapImage.java \
	AltosVoice.java \
	AltosDisplayThread.java \
	AltosDeviceUIDialog.java \
	AltosFreqList.java \
	AltosSerial.java \
	AltosSerialInUseException.java \
	AltosConfigFreqUI.java \
	AltosScanUI.java \
	AltosEepromDelete.java \
	AltosEepromManage.java \
	AltosEepromMonitorUI.java \
	AltosEepromSelect.java \
	AltosBTDevice.java \
	AltosBTDeviceIterator.java \
	AltosBTManage.java \
	AltosBTKnown.java


JAR=altosuilib_$(ALTOSUILIB_VERSION).jar

all-local: $(JAR)

clean-local:
	-rm -rf $(JAVAROOT) $(JAR)

install-altosuilibJAVA: $(JAR)
	@$(NORMAL_INSTALL)
	test -z "$(altosuilibdir)" || $(MKDIR_P) "$(DESTDIR)$(altosuilibdir)"
	echo " $(INSTALL_DATA)" "$(JAR)" "'$(DESTDIR)$(altosuilibdir)/$(JAR)"; \
	$(INSTALL_DATA) "$(JAR)" "$(DESTDIR)$(altosuilibdir)"

$(JAVAROOT):
	mkdir -p $(JAVAROOT)

$(JAR): classaltosuilib.stamp
	jar cf $@ -C $(JAVAROOT) .

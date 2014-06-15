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
	AltosVoice.java \
	AltosDisplayThread.java \
	AltosDeviceUIDialog.java \
	AltosFreqList.java \
	AltosReplaySpeed.java \
	AltosSerial.java \
	AltosSerialInUseException.java \
	AltosConfigFreqUI.java \
	AltosScanUI.java \
	AltosEepromDelete.java \
	AltosEepromManage.java \
	AltosEepromMonitorUI.java \
	AltosEepromSelect.java \
	AltosCSVUI.java \
	AltosDataChooser.java \
	AltosLights.java \
	AltosLed.java \
	AltosFlashUI.java \
	AltosRomconfigUI.java \
	AltosInfoTable.java \
	AltosFlightInfoTableModel.java \
	AltosFlightStatsTable.java \
	AltosGraph.java \
	AltosGraphDataPoint.java \
	AltosGraphDataSet.java \
	AltosBTDevice.java \
	AltosBTDeviceIterator.java \
	AltosBTManage.java \
	AltosBTKnown.java \
	AltosUIMap.java \
	AltosUIMapView.java \
	AltosUIMapLine.java \
	AltosUIMapMark.java \
	AltosUIMapPath.java \
	AltosUIMapTile.java \
	AltosUIMapCache.java \
	AltosUIMapImage.java \
	AltosUIMapTransform.java \
	AltosUIMapRectangle.java \
	AltosUIMapZoomListener.java \
	AltosUIMapTileListener.java \
	AltosUIMapPreload.java \
	AltosUIMapStore.java \
	AltosUIMapStoreListener.java \
	AltosUILatLon.java \
	AltosUIFlightTab.java \
	AltosUIIndicator.java \
	AltosUIUnitsIndicator.java \
	AltosUIVoltageIndicator.java

JAR=altosuilib_$(ALTOSUILIB_VERSION).jar

# Icons
ICONDIR=$(top_srcdir)/icon

ICONS=	$(ICONDIR)/redled.png $(ICONDIR)/redoff.png \
	$(ICONDIR)/greenled.png $(ICONDIR)/greenoff.png \
	$(ICONDIR)/grayon.png $(ICONDIR)/grayled.png

# icon base names for jar
ICONJAR= -C $(ICONDIR) redled.png -C $(ICONDIR) redoff.png \
	-C $(ICONDIR) greenled.png -C $(ICONDIR) greenoff.png \
	-C $(ICONDIR) grayon.png -C $(ICONDIR) grayled.png

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

$(JAR): classaltosuilib.stamp $(ICONS)
	jar cf $@ $(ICONJAR) -C $(JAVAROOT) .

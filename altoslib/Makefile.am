AM_JAVACFLAGS=-encoding UTF-8 -Xlint:deprecation

JAVAROOT=bin

VERSION=1

CLASSPATH_ENV=mkdir -p $(JAVAROOT); CLASSPATH="bin:$(FREETTS)/*:/usr/share/java/*"

SRC=.

altoslibdir = $(datadir)/java

altoslib_JAVA = \
	AltosLib.java \
	AltosConfigData.java \
	AltosConfigValues.java \
	AltosConvert.java \
	AltosCRCException.java \
	AltosDebug.java \
	AltosEeprom.java \
	AltosEepromChunk.java \
	AltosEepromFile.java \
	AltosEepromTM.java \
	AltosEepromHeader.java \
	AltosEepromIterable.java \
	AltosEepromLog.java \
	AltosEepromMega.java \
	AltosEepromMegaIterable.java \
	AltosEepromRecord.java \
	AltosEepromTeleScience.java \
	AltosEepromMini.java \
	AltosEepromOldIterable.java \
	AltosFile.java \
	AltosFlash.java \
	AltosFlashListener.java \
	AltosFlightReader.java \
	AltosFrequency.java \
	AltosGPS.java \
	AltosGPSQuery.java \
	AltosGPSSat.java \
	AltosGreatCircle.java \
	AltosHexfile.java \
	AltosIdleMonitor.java \
	AltosIdleMonitorListener.java \
	AltosIgnite.java \
	AltosIMU.java \
	AltosIMUQuery.java \
	AltosLine.java \
	AltosLink.java \
	AltosListenerState.java \
	AltosLog.java \
	AltosMs5607.java \
	AltosMs5607Query.java \
	AltosOrderedRecord.java \
	AltosOrderedMegaRecord.java \
	AltosOrderedMiniRecord.java \
	AltosParse.java \
	AltosPreferences.java \
	AltosPreferencesBackend.java \
	AltosRecordCompanion.java \
	AltosRecordIterable.java \
	AltosRecord.java \
	AltosRecordNone.java \
	AltosRecordTM.java \
	AltosRecordMM.java \
	AltosRecordMini.java \
	AltosReplayReader.java \
	AltosRomconfig.java \
	AltosSensorMM.java \
	AltosSensorTM.java \
	AltosState.java \
	AltosStateIterable.java \
	AltosStateUpdate.java \
	AltosTelemetry.java \
	AltosTelemetryFile.java \
	AltosTelemetryIterable.java \
	AltosTelemetryLegacy.java \
	AltosTelemetryMap.java \
	AltosTelemetryReader.java \
	AltosUnitsListener.java \
	AltosMs5607.java \
	AltosIMU.java \
	AltosMag.java \
	AltosUnits.java \
	AltosDistance.java \
	AltosHeight.java \
	AltosSpeed.java \
	AltosTemperature.java \
	AltosAccel.java \
	AltosPyro.java

JAR=altoslib_$(ALTOSLIB_VERSION).jar

all-local: $(JAR)

clean-local:
	-rm -rf bin $(JAR)

install-altoslibJAVA: $(JAR)
	@$(NORMAL_INSTALL)
	test -z "$(altoslibdir)" || $(MKDIR_P) "$(DESTDIR)$(altoslibdir)"
	echo " $(INSTALL_DATA)" "$<" "'$(DESTDIR)$(altoslibdir)/$(JAR)"; \
	$(INSTALL_DATA) "$<" "$(DESTDIR)$(altoslibdir)"

bin:
	mkdir -p bin

$(JAR): classaltoslib.stamp
	jar cf $@ -C bin org

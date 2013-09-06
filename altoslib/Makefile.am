AM_JAVACFLAGS=-encoding UTF-8 -Xlint:deprecation

JAVAROOT=bin

VERSION=1

CLASSPATH_ENV=mkdir -p $(JAVAROOT); CLASSPATH="bin:$(FREETTS)/*:/usr/share/java/*"

SRC=.

altoslibdir = $(datadir)/java
record_files = \
	AltosEepromRecord.java \
	AltosEepromTeleScience.java \
	AltosRecordCompanion.java \
	AltosRecordIterable.java \
	AltosOrderedRecord.java \
	AltosOrderedMegaRecord.java \
	AltosOrderedMiniRecord.java \
	AltosRecord.java \
	AltosRecordNone.java \
	AltosRecordTM.java \
	AltosRecordMM.java \
	AltosRecordMini.java


altoslib_JAVA = \
	AltosLib.java \
	AltosCompanion.java \
	AltosConfigData.java \
	AltosConfigValues.java \
	AltosConvert.java \
	AltosCRCException.java \
	AltosDebug.java \
	AltosEeprom.java \
	AltosEepromChunk.java \
	AltosEepromFile.java \
	AltosEepromTM.java \
	AltosEepromTm.java \
	AltosEepromHeader.java \
	AltosEepromIterable.java \
	AltosEepromLog.java \
	AltosEepromMega.java \
	AltosEepromMetrum2.java \
	AltosEepromMini.java \
	AltosFile.java \
	AltosFlash.java \
	AltosFlashListener.java \
	AltosFlightReader.java \
	AltosFrequency.java \
	AltosGPS.java \
	AltosGPSSat.java \
	AltosGreatCircle.java \
	AltosHexfile.java \
	AltosIdle.java \
	AltosIdleFetch.java \
	AltosIdleMonitor.java \
	AltosIdleMonitorListener.java \
	AltosIgnite.java \
	AltosIMU.java \
	AltosLine.java \
	AltosLink.java \
	AltosListenerState.java \
	AltosLog.java \
	AltosMag.java \
	AltosMma655x.java \
	AltosMs5607.java \
	AltosParse.java \
	AltosPreferences.java \
	AltosPreferencesBackend.java \
	AltosReplayReader.java \
	AltosRomconfig.java \
	AltosSensorMM.java \
	AltosSensorEMini.java \
	AltosSensorTM.java \
	AltosSensorMega.java \
	AltosSensorMetrum.java \
	AltosState.java \
	AltosStateIterable.java \
	AltosStateUpdate.java \
	AltosTelemetry.java \
	AltosTelemetryConfiguration.java \
	AltosTelemetryFile.java \
	AltosTelemetryIterable.java \
	AltosTelemetryLegacy.java \
	AltosTelemetryLocation.java \
	AltosTelemetryMap.java \
	AltosTelemetryMegaSensor.java \
	AltosTelemetryMegaData.java \
	AltosTelemetryMetrumSensor.java \
	AltosTelemetryMetrumData.java \
	AltosTelemetryReader.java \
	AltosTelemetryRaw.java \
	AltosTelemetrySensor.java \
	AltosTelemetrySatellite.java \
	AltosTelemetryStandard.java \
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

AM_JAVACFLAGS=-encoding UTF-8 -Xlint:deprecation

JAVAROOT=bin

CLASSPATH_ENV=mkdir -p $(JAVAROOT); CLASSPATH="bin:$(FREETTS)/*:/usr/share/java/*"

SRC=.
BIN=bin/org/altusmetrum/AltosLib

AltosLibdir = $(datadir)/java

AltosLib_JAVA = \
	$(SRC)/AltosLib.java \
	$(SRC)/AltosConfigData.java \
	$(SRC)/AltosConvert.java \
	$(SRC)/AltosCRCException.java \
	$(SRC)/AltosEepromChunk.java \
	$(SRC)/AltosEepromIterable.java \
	$(SRC)/AltosEepromLog.java \
	$(SRC)/AltosEepromMega.java \
	$(SRC)/AltosEepromMegaIterable.java \
	$(SRC)/AltosEepromRecord.java \
	$(SRC)/AltosEepromTeleScience.java \
	$(SRC)/AltosFile.java \
	$(SRC)/AltosFlightReader.java \
	$(SRC)/AltosFrequency.java \
	$(SRC)/AltosGPS.java \
	$(SRC)/AltosGPSQuery.java \
	$(SRC)/AltosGPSSat.java \
	$(SRC)/AltosGreatCircle.java \
	$(SRC)/AltosIdleMonitor.java \
	$(SRC)/AltosIdleMonitorListener.java \
	$(SRC)/AltosIgnite.java \
	$(SRC)/AltosIMU.java \
	$(SRC)/AltosIMUQuery.java \
	$(SRC)/AltosLine.java \
	$(SRC)/AltosLink.java \
	$(SRC)/AltosLog.java \
	$(SRC)/AltosMs5607.java \
	$(SRC)/AltosMs5607Query.java \
	$(SRC)/AltosOrderedRecord.java \
	$(SRC)/AltosOrderedMegaRecord.java \
	$(SRC)/AltosParse.java \
	$(SRC)/AltosPreferences.java \
	$(SRC)/AltosRecordCompanion.java \
	$(SRC)/AltosRecordIterable.java \
	$(SRC)/AltosRecord.java \
	$(SRC)/AltosRecordTM.java \
	$(SRC)/AltosRecordMM.java \
	$(SRC)/AltosReplayReader.java \
	$(SRC)/AltosSensorMM.java \
	$(SRC)/AltosSensorTM.java \
	$(SRC)/AltosState.java \
	$(SRC)/AltosTelemetry.java \
	$(SRC)/AltosTelemetryIterable.java \
	$(SRC)/AltosTelemetryMap.java \
	$(SRC)/AltosTelemetryReader.java \
	$(SRC)/AltosTelemetryRecordCompanion.java \
	$(SRC)/AltosTelemetryRecordConfiguration.java \
	$(SRC)/AltosTelemetryRecordGeneral.java \
	$(SRC)/AltosTelemetryRecord.java \
	$(SRC)/AltosTelemetryRecordLegacy.java \
	$(SRC)/AltosTelemetryRecordLocation.java \
	$(SRC)/AltosTelemetryRecordRaw.java \
	$(SRC)/AltosTelemetryRecordSatellite.java \
	$(SRC)/AltosTelemetryRecordSensor.java \
	$(SRC)/AltosTelemetryRecordMegaSensor.java \
	$(SRC)/AltosTelemetryRecordMegaData.java \
	$(SRC)/AltosMs5607.java \
	$(SRC)/AltosIMU.java \
	$(SRC)/AltosMag.java \
	$(SRC)/AltosUnits.java \
	$(SRC)/AltosDistance.java \
	$(SRC)/AltosHeight.java \
	$(SRC)/AltosSpeed.java \
	$(SRC)/AltosAccel.java

JAR=AltosLib.jar

all-local: $(JAR)

clean-local:
	-rm -rf bin $(JAR)

install-AltosLibJAVA: $(JAR)
	@$(NORMAL_INSTALL)
	test -z "$(AltosLibdir)" || $(MKDIR_P) "$(DESTDIR)$(AltosLibdir)"
	echo " $(INSTALL_DATA)" "$<" "'$(DESTDIR)$(AltosLibdir)/$(JAR)"; \
	$(INSTALL_DATA) "$<" "$(DESTDIR)$(AltosLibdir)"

bin:
	mkdir -p bin

$(JAR): classAltosLib.stamp
	jar cf $@ -C bin org

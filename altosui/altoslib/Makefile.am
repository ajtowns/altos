AM_JAVACFLAGS=-encoding UTF-8 -Xlint:deprecation

JAVAROOT=bin

CLASSPATH_ENV=mkdir -p $(JAVAROOT); CLASSPATH="bin:$(FREETTS)/*:/usr/share/java/*"

SRC=src/org/altusmetrum/AltosLib
BIN=bin/org/altusmetrum/AltosLib

AltosLibdir = $(datadir)/java

AltosLib_JAVA = \
	$(SRC)/AltosLib.java \
	$(SRC)/AltosConfigData.java \
	$(SRC)/AltosConvert.java \
	$(SRC)/AltosCRCException.java \
	$(SRC)/AltosEepromChunk.java \
	$(SRC)/AltosEepromIterable.java \
	$(SRC)/AltosEepromRecord.java \
	$(SRC)/AltosFrequency.java \
	$(SRC)/AltosGPS.java \
	$(SRC)/AltosGPSSat.java \
	$(SRC)/AltosLine.java \
	$(SRC)/AltosLink.java \
	$(SRC)/AltosParse.java \
	$(SRC)/AltosPreferences.java \
	$(SRC)/AltosRecordCompanion.java \
	$(SRC)/AltosRecordIterable.java \
	$(SRC)/AltosRecord.java \
	$(SRC)/AltosTelemetry.java \
	$(SRC)/AltosTelemetryIterable.java \
	$(SRC)/AltosTelemetryMap.java \
	$(SRC)/AltosTelemetryRecordCompanion.java \
	$(SRC)/AltosTelemetryRecordConfiguration.java \
	$(SRC)/AltosTelemetryRecordGeneral.java \
	$(SRC)/AltosTelemetryRecord.java \
	$(SRC)/AltosTelemetryRecordLegacy.java \
	$(SRC)/AltosTelemetryRecordLocation.java \
	$(SRC)/AltosTelemetryRecordRaw.java \
	$(SRC)/AltosTelemetryRecordSatellite.java \
	$(SRC)/AltosTelemetryRecordSensor.java

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

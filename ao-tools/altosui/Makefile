.SUFFIXES: .java .class

CLASSPATH=classes:./*
CLASSFILES=\
	Altos.class \
	AltosChannelMenu.class \
	AltosConfig.class \
	AltosConfigUI.class \
	AltosConvert.class \
	AltosCSV.class \
	AltosEepromDownload.class \
	AltosEepromMonitor.class \
	AltosEepromReader.class \
	AltosEepromRecord.class \
	AltosFile.class \
	AltosFlash.class \
	AltosFlightInfoTableModel.class \
	AltosFlightStatusTableModel.class \
	AltosGPS.class \
	AltosGreatCircle.class \
	AltosHexfile.class \
	AltosLog.class \
	AltosParse.class \
	AltosPreferences.class \
	AltosRecord.class \
	AltosSerialMonitor.class \
	AltosSerial.class \
	AltosState.class \
	AltosTelemetry.class \
	AltosTelemetryReader.class \
	AltosUI.class \
	AltosDevice.class \
	AltosDeviceDialog.class \
	AltosVoice.class

FREETTSSRC=/home/keithp/src/freetts/freetts-1.2.2
FREETTSLIB=$(FREETTSSRC)/lib
FREETTSJAR= \
	cmudict04.jar \
	cmulex.jar \
	cmu_time_awb.jar \
	cmutimelex.jar \
	cmu_us_kal.jar \
	en_us.jar \
	freetts.jar

JAVAFLAGS=-Xlint:unchecked -Xlint:deprecation

OS:=$(shell uname)

ifeq ($(OS),Linux)
ALTOSUI_APP=altosui
endif

ifeq ($(OS),Darwin)
ALTOSUI_APP=AltosUI.app/Contents/Resources/Java/altosui.jar
endif

all: altosui.jar $(ALTOSUI_APP)

$(CLASSFILES):

.java.class:
	javac -encoding UTF8 -classpath "$(CLASSPATH)" $(JAVAFLAGS) $*.java

altosui.jar: classes/altosui classes/libaltosJNI $(FREETTSJAR) $(CLASSFILES) Manifest.txt
	cd ./classes && jar cfm ../$@ altosui/Manifest.txt altosui/*.class libaltosJNI/*.class

classes/altosui:
	mkdir -p classes
	ln -s .. classes/altosui

classes/libaltosJNI:
	mkdir -p classes
	ln -s ../../libaltos/libaltosJNI classes/libaltosJNI

$(FREETTSJAR):
	ln -s $(FREETTSLIB)/$@ .

ifeq ($(OS),Darwin)
RESOURCES=altosui.jar $(FREETTSJAR) ../libaltos/libaltos.dylib

$(ALTOSUI_APP): $(RESOURCES)
	mkdir -p AltosUI.app/Contents/Resources/Java
	cp $(RESOURCES) AltosUI.app/Contents/Resources/Java

endif

ifeq ($(OS),Linux)
altosui:
	echo "#!/bin/sh" > $@
	echo "exec java -Djava.library.path=../libaltos -jar altosui.jar" >> $@
	chmod +x ./altosui
endif

clean:
	rm -f *.class $(FREETTSJAR) altosui.jar
	rm -f AltosUI.app/Contents/Resources/Java/*
	rm -rf classes

.SUFFIXES: .java .class

CLASSPATH=..:/usr/share/java/*:/home/keithp/src/freetts/freetts-1.2.2
CLASSFILES=\
	AltosConvert.class \
	AltosFile.class \
	AltosGPS.class \
	AltosGreatCircle.class \
	AltosLog.class \
	AltosParse.class \
	AltosPreferences.class \
	AltosSerialMonitor.class \
	AltosSerial.class \
	AltosState.class \
	AltosTelemetry.class \
	AltosUI.class \
	AltosDevice.class \
	AltosDeviceLinux.class \
	AltosDeviceDialog.class \
	AltosVoice.class

JAVAFLAGS=-Xlint:unchecked

all: $(CLASSFILES) altosui altosui.jar

.java.class:
	javac -cp "$(CLASSPATH)" $(JAVAFLAGS) $*.java

altosui: Makefile
	(echo '#!/bin/sh'; \
	echo exec java -cp '"$(CLASSPATH)"' altosui/AltosUI) > $@
	chmod +x $@

altosui.jar: $(CLASSFILES) Manifest.txt
	cd .. && jar cfm altosui/$@ altosui/Manifest.txt altosui/*.class

clean:
	rm -f *.class

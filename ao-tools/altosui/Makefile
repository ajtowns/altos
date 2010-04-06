.SUFFIXES: .java .class

CLASSPATH=..:/usr/share/java/*
CLASSFILES=\
	AltosConvert.class \
	AltosGPS.class \
	AltosGreatCircle.class \
	AltosParse.class \
	AltosSerialMonitor.class \
	AltosSerial.class \
	AltosState.class \
	AltosTelemetry.class \
	AltosUI.class \
	AltosDevice.class \
	AltosDeviceLinux.class \

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

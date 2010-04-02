.SUFFIXES: .java .class

CLASSPATH=..:/usr/share/java/*
CLASSFILES=AltosSerialMonitor.class AltosSerial.class AltosTelemetry.class AltosUI.class
JAVAFLAGS=-Xlint:unchecked

all: $(CLASSFILES) altosui

.java.class:
	javac -cp "$(CLASSPATH)" $(JAVAFLAGS) $*.java

altosui: Makefile
	(echo '#!/bin/sh'; \
	echo exec java -cp '"$(CLASSPATH)"' altosui/AltosUI) > $@
	chmod +x $@

clean:
	rm -f *.class
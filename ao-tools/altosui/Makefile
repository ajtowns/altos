.SUFFIXES: .java .class

CLASSPATH=..:../libaltos:/usr/share/java/*:/Users/keithp/freetts-1.2.2/lib/*
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
	AltosDeviceDialog.class \
	AltosVoice.class

JAVAFLAGS=-Xlint:unchecked

all: $(CLASSFILES) altosui.jar

.java.class:
	javac -cp "$(CLASSPATH)" $(JAVAFLAGS) $*.java

altosui.jar: $(CLASSFILES) Manifest.txt
	jar cfm $@ altosui/Manifest.txt altosui/*.class libaltosJNI/*.class

clean:
	rm -f *.class

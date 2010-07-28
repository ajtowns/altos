.SUFFIXES: .java .class

CLASSPATH=classes:./*
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

FREETTSSRC=/home/keithp/src/freetts/freetts-1.2.2
FREETTSLIB=$(FREETTSSRC)/lib
FREETTSJAR= \
	cmudict04.jar \
	cmulex.jar \
	cmu_time_awb.jar \
	cmutimelex.jar \
	cmu_us_kal.jar \
	en_us.jar \
	freetts.jar \
	freetts-jsapi10.jar \
	jsapi.jar

JAVAFLAGS=-Xlint:unchecked

all: altosui.jar

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

clean:
	rm -f *.class $(FREETTSJAR) altosui.jar
	rm -rf classes

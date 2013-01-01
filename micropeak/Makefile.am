JAVAROOT=classes
AM_JAVACFLAGS=-encoding UTF-8 -Xlint:deprecation

CLASSPATH_ENV=mkdir -p $(JAVAROOT); CLASSPATH=".:classes:../altoslib/*:../altosuilib/*:../libaltos:$(JCOMMON)/jcommon.jar:$(JFREECHART)/jfreechart.jar"

bin_SCRIPTS=micropeak

micropeakdir=$(datadir)/java

micropeak_JAVA= \
	MicroPeak.java \
	MicroData.java \
	MicroDownload.java \
	MicroFrame.java \
	MicroGraph.java \
	MicroSave.java \
	MicroSerial.java \
	MicroStats.java \
	MicroStatsTable.java \
	MicroFileChooser.java \
	MicroDeviceDialog.java \
	MicroUSB.java

JFREECHART_CLASS= \
    jfreechart.jar

JCOMMON_CLASS=\
    jcommon.jar

JAR=micropeak.jar

FATJAR=micropeak-fat.jar

LIBALTOS= \
	libaltos.so \
	libaltos.dylib \
	altos.dll

ALTOSLIB_CLASS=\
	AltosLib.jar

ALTOSUILIB_CLASS=\
	AltosUILib.jar

# Icons
ICONDIR=$(top_srcdir)/icon

JAVA_ICONS=\
	$(ICONDIR)/micropeak-16.png \
	$(ICONDIR)/micropeak-32.png \
	$(ICONDIR)/micropeak-48.png \
	$(ICONDIR)/micropeak-64.png \
	$(ICONDIR)/micropeak-128.png \
	$(ICONDIR)/micropeak-256.png

# icon base names for jar
ICONJAR= -C $(ICONDIR) micropeak-16.png \
	-C $(ICONDIR) micropeak-32.png \
	-C $(ICONDIR) micropeak-48.png \
	-C $(ICONDIR) micropeak-64.png \
	-C $(ICONDIR) micropeak-128.png \
	-C $(ICONDIR) micropeak-256.png

all-local: micropeak-test $(JAR)

clean-local:
	-rm -rf classes $(JAR) $(FATJAR) \
		$(ALTOSLIB_CLASS) \
		$(ALTOSUILIB_CLASS) \
		$(JFREECHART_CLASS) $(JCOMMON_CLASS) $(LIBALTOS) Manifest.txt \
		micropeak micropeak-test macosx linux windows

micropeak: Makefile
	echo "#!/bin/sh" > $@
	echo 'exec java  -cp "$(JCOMMON)/jcommon.jar:$(JFREECHART)/jfreechart.jar" -Djava.library.path="$(altoslibdir)" -jar "$(micropeakdir)/micropeak.jar" "$$@"' >> $@
	chmod +x $@

micropeak-test: Makefile
	echo "#!/bin/sh" > $@
	echo 'exec java -cp "./*:../libaltos/*:$(JCOMMON)/jcommon.jar:$(JFREECHART)/jfreechart.jar" -Djava.library.path="../libaltos/.libs" -jar micropeak.jar "$$@"' >> $@
	chmod +x $@

$(JAR): classmicropeak.stamp Manifest.txt $(JAVA_ICONS) $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS)
	jar cfm $@ Manifest.txt \
		$(ICONJAR) \
		-C classes org \
		-C ../libaltos libaltosJNI

$(FATJAR): classmicropeak.stamp Manifest-fat.txt $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS) $(JFREECHART_CLASS) $(JCOMMON_CLASS) $(JAVA_ICONS)
	jar cfm $@ Manifest-fat.txt \
		$(ICONJAR) \
		-C classes org \
		-C ../libaltos libaltosJNI


libaltos.so: build-libaltos
	-rm -f "$@"
	$(LN_S) ../libaltos/.libs/"$@" .

libaltos.dylib:
	-rm -f "$@"
	$(LN_S) ../libaltos/"$@" .

altos.dll: ../libaltos/altos.dll
	-rm -f "$@"
	$(LN_S) ../libaltos/"$@" .

altos64.dll: ../libaltos/altos64.dll
	-rm -f "$@"
	$(LN_S) ../libaltos/"$@" .

../libaltos/.libs/libaltos.so: build-libaltos

../libaltos/altos.dll: build-altos-dll

../libaltos/altos64.dll: build-altos64-dll

build-libaltos:
	+cd ../libaltos && make libaltos.la
build-altos-dll:
	+cd ../libaltos && make altos.dll

build-altos64-dll:
	+cd ../libaltos && make altos64.dll

$(ALTOSLIB_CLASS):
	-rm -f "$@"
	$(LN_S) ../altoslib/"$@" .

$(ALTOSUILIB_CLASS):
	-rm -f "$@"
	$(LN_S) ../altosuilib/"$@" .

$(JFREECHART_CLASS):
	-rm -f "$@"
	$(LN_S) "$(JFREECHART)"/"$@" .

$(JCOMMON_CLASS):
	-rm -f "$@"
	$(LN_S) "$(JCOMMON)"/"$@" .

Manifest.txt: Makefile
	echo 'Main-Class: org.altusmetrum.micropeak.MicroPeak' > $@
	echo "Class-Path: AltosLib.jar AltosUILib.jar $(JCOMMON)/jcommon.jar $(JFREECHART)/jfreechart.jar" >> $@

Manifest-fat.txt:
	echo 'Main-Class: org.altusmetrum.micropeak.MicroPeak' > $@
	echo "Class-Path: AltosLib.jar AltosUILib.jar jcommon.jar jfreechart.jar" >> $@


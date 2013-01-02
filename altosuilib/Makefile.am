AM_JAVACFLAGS=-encoding UTF-8 -Xlint:deprecation

JAVAROOT=bin

CLASSPATH_ENV=mkdir -p $(JAVAROOT); CLASSPATH="bin:../altoslib/*:../libaltos:$(FREETTS)/*:/usr/share/java/*"

SRC=.
BIN=bin/org/altusmetrum/AltosUILib

AltosUILibdir = $(datadir)/java

AltosUILib_JAVA = \
	AltosUIConfigure.java \
	AltosDevice.java \
	AltosDeviceDialog.java \
	AltosUSBDevice.java \
	AltosFontListener.java \
	AltosUIDialog.java \
	AltosUIFrame.java \
	AltosUILib.java \
	AltosUIListener.java \
	AltosUIPreferencesBackend.java \
	AltosUIPreferences.java \
	AltosUIVersion.java \
	AltosUnitsListener.java

JAR=AltosUILib.jar

all-local: $(JAR)

clean-local:
	-rm -rf bin $(JAR)

install-AltosUILibJAVA: $(JAR)
	@$(NORMAL_INSTALL)
	test -z "$(AltosUILibdir)" || $(MKDIR_P) "$(DESTDIR)$(AltosUILibdir)"
	echo " $(INSTALL_DATA)" "$<" "'$(DESTDIR)$(AltosUILibdir)/$(JAR)"; \
	$(INSTALL_DATA) "$<" "$(DESTDIR)$(AltosUILibdir)"

bin:
	mkdir -p bin

$(JAR): classAltosUILib.stamp
	jar cf $@ -C bin org

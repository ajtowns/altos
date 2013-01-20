AM_JAVACFLAGS=-encoding UTF-8 -Xlint:deprecation

JAVAROOT=bin

CLASSPATH_ENV=mkdir -p $(JAVAROOT); CLASSPATH="$(JAVAROOT):../altoslib/*:../libaltos:/usr/share/java/*"

SRC=.

altosuilibdir = $(datadir)/java

altosuilib_JAVA = \
	AltosUIConfigure.java \
	AltosDevice.java \
	AltosDeviceDialog.java \
	AltosUSBDevice.java \
	AltosFontListener.java \
	AltosPositionListener.java \
	AltosUIDialog.java \
	AltosUIFrame.java \
	AltosUILib.java \
	AltosUIListener.java \
	AltosUIPreferencesBackend.java \
	AltosUIPreferences.java \
	AltosUIVersion.java \
	AltosUnitsListener.java

JAR=altosuilib.jar

all-local: $(JAR)

clean-local:
	-rm -rf $(JAVAROOT) $(JAR)

install-altosuilibJAVA: $(JAR)
	@$(NORMAL_INSTALL)
	test -z "$(altosuilibdir)" || $(MKDIR_P) "$(DESTDIR)$(altosuilibdir)"
	echo " $(INSTALL_DATA)" "$(JAR)" "'$(DESTDIR)$(altosuilibdir)/$(JAR)"; \
	$(INSTALL_DATA) "$(JAR)" "$(DESTDIR)$(altosuilibdir)"

$(JAVAROOT):
	mkdir -p $(JAVAROOT)

$(JAR): classaltosuilib.stamp
	jar cf $@ -C $(JAVAROOT) .

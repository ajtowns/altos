if ANDROID
all_target=bin/AltosDroid-debug.apk bin/AltosDroid-release.apk
clean_command=ant clean
else
all_target=
clean_command=echo done
endif

SDK=$(ANDROID_SDK)

DX=$(SDK)/platform-tools/dx
ADB=$(SDK)/platform-tools/adb
AAPT=$(SDK)/platform-tools/aapt
APKBUILDER=$(SDK)/tools/apkbuilder
ZIPALIGN=$(SDK)/tools/zipalign

SRC_DIR=src/org/altusmetrum/AltosDroid
EXT_LIBDIR=libs
ALTOSLIB_SRCDIR=../altoslib
ALTOSLIB_JAR=AltosLib.jar

ALTOSLIB=$(EXT_LIBDIR)/$(ALTOSLIB_JAR)

SRC=\
	$(SRC_DIR)/AltosDroid.java \
	$(SRC_DIR)/TelemetryService.java \
	$(SRC_DIR)/TelemetryReader.java \
	$(SRC_DIR)/AltosBluetooth.java \
	$(SRC_DIR)/DeviceListActivity.java \
	$(SRC_DIR)/Dumper.java

all: $(all_target)

$(ALTOSLIB): $(ALTOSLIB_SRCDIR)/$(ALTOSLIB_JAR)
	mkdir -p $(EXT_LIBDIR)
	cd $(EXT_LIBDIR) && ln -s $(shell echo $(EXT_LIBDIR) | sed 's|[^/]\+|..|g')/$(ALTOSLIB_SRCDIR)/$(ALTOSLIB_JAR) .

if ANDROID
install-release: bin/AltosDroid-release.apk
	$(ADB) install -r bin/AltosDroid-release.apk

install-debug: bin/AltosDroid-debug.apk
	$(ADB) install -r bin/AltosDroid-debug.apk

bin/AltosDroid-debug.apk: $(SRC) $(ALTOSLIB)
	ant debug

bin/AltosDroid-release.apk: $(SRC) $(ALTOSLIB)
	ant release
endif

clean:
	$(clean_command)


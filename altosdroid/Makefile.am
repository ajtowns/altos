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

SRC=\
	$(SRC_DIR)/AltosDroid.java \
	$(SRC_DIR)/TelemetryService.java \
	$(SRC_DIR)/TelemetryServiceActivities.java \
	$(SRC_DIR)/BluetoothChatService.java \
	$(SRC_DIR)/DeviceListActivity.java

all: $(all_target)

ALTOSLIB=bin/classes/AltosLib.jar

$(ALTOSLIB):
	mkdir -p bin/classes
	cd bin/classes && ln -s ../../../altoslib/AltosLib.jar .

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


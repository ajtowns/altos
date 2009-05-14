#
# AltOS build
#
# 
CC=sdcc

VERSION=$(shell git describe)

CFLAGS=--model-small --debug --opt-code-speed 

LDFLAGS=--out-fmt-ihx --code-loc 0x0000 --code-size 0x8000 \
	--xram-loc 0xf000 --xram-size 0xda2 --iram-size 0xff

SERIAL=1

INC = \
	ao.h \
	cc1111.h \
	altitude.h \
	25lc1024.h

#
# Common AltOS sources
#
ALTOS_SRC = \
	ao_cmd.c \
	ao_dbg.c \
	ao_dma.c \
	ao_mutex.c \
	ao_panic.c \
	ao_task.c \
	ao_timer.c \
	_bp.c

#
# Shared AltOS drivers
#
ALTOS_DRIVER_SRC = \
	ao_beep.c \
	ao_config.c \
	ao_led.c \
	ao_radio.c \
	ao_stdio.c \
	ao_usb.c

TELE_COMMON_SRC = \
	ao_gps_print.c \
	ao_state.c

#
# Receiver code
#
TELE_RECEIVER_SRC =\
	ao_monitor.c \
	ao_rssi.c

#
# Shared Tele drivers (on TeleMetrum, TeleTerra, TeleDongle)
#

TELE_DRIVER_SRC = \
	ao_convert.c \
	ao_gps.c \
	ao_serial.c

#
# Drivers for partially-flled boards (TT, TD and TI)
#
TELE_FAKE_SRC = \
	ao_adc_fake.c \
	ao_ee_fake.c

# 
# Drivers only on TeleMetrum
#
TM_DRIVER_SRC = \
	ao_adc.c \
	ao_ee.c \
	ao_gps_report.c \
	ao_ignite.c

#
# Tasks run on TeleMetrum
#
TM_TASK_SRC = \
	ao_flight.c \
	ao_log.c \
	ao_report.c \
	ao_telemetry.c

TM_MAIN_SRC = \
	ao_telemetrum.c

#
# All sources for TeleMetrum
#
TM_SRC = \
	$(ALTOS_SRC) \
	$(ALTOS_DRIVER_SRC) \
	$(TELE_DRIVER_SRC) \
	$(TELE_COMMON_SRC) \
	$(TM_DRIVER_SRC) \
	$(TM_TASK_SRC) \
	$(TM_MAIN_SRC)

TI_MAIN_SRC = \
	ao_tidongle.c

#
# All sources for the TI debug dongle
#
TI_SRC = \
	$(ALTOS_SRC) \
	$(ALTOS_DRIVER_SRC) \
	$(TELE_RECEIVER_SRC) \
	$(TELE_COMMON_SRC) \
	$(TELE_FAKE_SRC) \
	$(TI_MAIN_SRC)
	
TT_MAIN_SRC = \
	ao_teleterra.c
#
# All sources for TeleTerra
#
TT_SRC = \
	$(ALTOS_SRC) \
	$(ALTOS_DRIVER_SRC) \
	$(TELE_RECEIVER_SRC) \
	$(TELE_DRIVER_SRC) \
	$(TELE_COMMON_SRC) \
	$(TELE_FAKE_SRC) \
	$(TT_MAIN_SRC)
	
	
#
# Sources for TeleDongle
#

TD_MAIN_SRC = \
	ao_teledongle.c

TD_SRC = \
	$(ALTOS_SRC) \
	$(ALTOS_DRIVER_SRC) \
	$(TELE_RECEIVER_SRC) \
	$(TELE_COMMON_SRC) \
	$(TELE_FAKE_SRC) \
	$(TD_MAIN_SRC)

SRC = \
	$(ALTOS_SRC) \
	$(ALTOS_DRIVER_SRC) \
	$(TELE_DRIVER_SRC) \
	$(TELE_RECEIVER_SRC) \
	$(TELE_COMMON_SRC) \
	$(TELE_FAKE_SRC) \
	$(TM_DRIVER_SRC) \
	$(TM_TASK_SRC) \
	$(TM_MAIN_SRC) \
	$(TI_MAIN_SRC) \
	$(TD_MAIN_SRC) \
	$(TT_MAIN_SRC)

TM_REL=$(TM_SRC:.c=.rel) ao_product-telemetrum-$(SERIAL).rel
TI_REL=$(TI_SRC:.c=.rel) ao_product-tidongle-$(SERIAL).rel
TT_REL=$(TT_SRC:.c=.rel) ao_product-teleterra-$(SERIAL).rel
TD_REL=$(TD_SRC:.c=.rel) ao_product-teledongle-$(SERIAL).rel

PROD_REL=\
	ao_product-telemetrum-$(SERIAL).rel \
	ao_product-tidongle-$(SERIAL).rel \
	ao_product-teleterra-$(SERIAL).rel \
	ao_product-teledongle-$(SERIAL).rel

REL=$(SRC:.c=.rel) $(PROD_REL)
ADB=$(REL:.rel=.adb)
ASM=$(REL:.rel=.asm)
LNK=$(REL:.rel=.lnk)
LST=$(REL:.rel=.lst)
RST=$(REL:.rel=.rst)
SYM=$(REL:.rel=.sym)

PROGS=	telemetrum-$(SERIAL).ihx tidongle-$(SERIAL).ihx \
	teleterra-$(SERIAL).ihx teledongle-$(SERIAL).ihx

HOST_PROGS=ao_flight_test

PCDB=$(PROGS:.ihx=.cdb)
PLNK=$(PROGS:.ihx=.lnk)
PMAP=$(PROGS:.ihx=.map)
PMEM=$(PROGS:.ihx=.mem)
PAOM=$(PROGS:.ihx=)

%.rel : %.c $(INC)
	$(CC) -c $(CFLAGS) -o$*.rel $*.c

all: $(PROGS) $(HOST_PROGS)

telemetrum-$(SERIAL).ihx: $(TM_REL) Makefile
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(TM_REL)
	sh check-stack ao.h telemetrum-$(SERIAL).mem

tidongle-$(SERIAL).ihx: $(TI_REL) Makefile
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(TI_REL)
	sh check-stack ao.h tidongle-$(SERIAL).mem

tidongle-$(SERIAL).ihx: telemetrum-$(SERIAL).ihx

teleterra-$(SERIAL).ihx: $(TT_REL) Makefile
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(TT_REL)
	sh check-stack ao.h teleterra-$(SERIAL).mem

teleterra-$(SERIAL).ihx: tidongle-$(SERIAL).ihx

teledongle-$(SERIAL).ihx: $(TD_REL) Makefile
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(TD_REL)
	sh check-stack ao.h teledongle-$(SERIAL).mem

teledongle-$(SERIAL).ihx: teleterra-$(SERIAL).ihx

altitude.h: make-altitude
	nickle make-altitude > altitude.h

TELEMETRUM_DEFS=ao-telemetrum-$(SERIAL).h
TELETERRA_DEFS=ao-teleterra-$(SERIAL).h
TELEDONGLE_DEFS=ao-teledongle-$(SERIAL).h
TIDONGLE_DEFS=ao-tidongle-$(SERIAL).h

ALL_DEFS=$(TELEMETRUM_DEFS) $(TELETERRA_DEFS) \
	$(TELEDONGLE_DEFS) $(TIDONGLE_DEFS)
ao_product-telemetrum-$(SERIAL).rel: ao_product.c $(TELEMETRUM_DEFS)
	$(CC) -c $(CFLAGS) -D PRODUCT_DEFS='\"$(TELEMETRUM_DEFS)\"' -o$@ ao_product.c

ao_product-teleterra-$(SERIAL).rel: ao_product.c $(TELETERRA_DEFS)
	$(CC) -c $(CFLAGS) -D PRODUCT_DEFS='\"$(TELETERRA_DEFS)\"' -o$@ ao_product.c

ao_product-teledongle-$(SERIAL).rel: ao_product.c $(TELEDONGLE_DEFS)
	$(CC) -c $(CFLAGS) -D PRODUCT_DEFS='\"$(TELEDONGLE_DEFS)\"' -o$@ ao_product.c

ao_product-tidongle-$(SERIAL).rel: ao_product.c $(TIDONGLE_DEFS)
	$(CC) -c $(CFLAGS) -D PRODUCT_DEFS='\"$(TIDONGLE_DEFS)\"' -o$@ ao_product.c

$(TELEMETRUM_DEFS): ao-make-product.5c
	nickle ao-make-product.5c -m altusmetrum.org -p TeleMetrum -s $(SERIAL) -v $(VERSION) > $@

$(TELETERRA_DEFS): ao-make-product.5c
	nickle ao-make-product.5c -m altusmetrum.org -p TeleTerra -s $(SERIAL) -v $(VERSION) > $@

$(TELEDONGLE_DEFS): ao-make-product.5c
	nickle ao-make-product.5c -m altusmetrum.org -p TeleDongle -s $(SERIAL) -v $(VERSION) > $@

$(TIDONGLE_DEFS): ao-make-product.5c
	nickle ao-make-product.5c -m altusmetrum.org -p TIDongle -s $(SERIAL) -v $(VERSION) > $@

clean:
	rm -f $(ADB) $(ASM) $(LNK) $(LST) $(REL) $(RST) $(SYM)
	rm -f $(PROGS) $(PCDB) $(PLNK) $(PMAP) $(PMEM) $(PAOM)
	rm -f $(ALL_DEFS) $(HOST_PROGS)
	rm -f $(TELEMETRUM_DEFS) $(TELETERRA_DEFS) $(TELEDONGLE_DEFS) $(TIDONGLE_DEFS)

install:

ao_flight_test: ao_flight.c ao_flight_test.c
	cc -g -o $@ ao_flight_test.c

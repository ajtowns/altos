#
# AltOS build
#
# 
CC=/usr/bin/sdcc

CFLAGS=--model-small --debug --opt-code-speed 

LDFLAGS=--out-fmt-ihx --code-loc 0x0000 --code-size 0x8000 \
	--xram-loc 0xf000 --xram-size 0xf00 --iram-size 0xff

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
	ao_led.c \
	ao_radio.c \
	ao_stdio.c \
	ao_usb.c

TELE_COMMON_SRC = \
	ao_gps_print.c

#
# Receiver code
#
TELE_RECEIVER_SRC =\
	ao_monitor.c

#
# Shared Tele drivers (on TeleMetrum, TeleTerra, TeleDongle)
#

TELE_DRIVER_SRC = \
	ao_convert.c \
	ao_gps.c \
	ao_serial.c

# 
# Drivers only on TeleMetrum
#
TM_DRIVER_SRC = \
	ao_adc.c \
	ao_ee.c

#
# Tasks run on TeleMetrum
#
TM_TASK_SRC = \
	ao_flight.c \
	ao_log.c \
	ao_report.c \
	ao_telemetry.c \
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
	$(TM_TASK_SRC)

TI_TASK_SRC = \
	ao_tidongle.c

#
# All sources for the TI debug dongle
#
TI_SRC = \
	$(ALTOS_SRC) \
	$(ALTOS_DRIVER_SRC) \
	$(TELE_RECEIVER_SRC) \
	$(TELE_COMMON_SRC) \
	$(TI_TASK_SRC)
	
SRC = \
	$(ALTOS_SRC) \
	$(ALTOS_DRIVER_SRC) \
	$(TELE_DRIVER_SRC) \
	$(TELE_RECEIVER_SRC) \
	$(TELE_COMMON_SRC) \
	$(TM_DRIVER_SRC) \
	$(TM_TASK_SRC) \
	$(TI_TASK_SRC)

TM_REL=$(TM_SRC:.c=.rel)
TI_REL=$(TI_SRC:.c=.rel)

ADB=$(SRC:.c=.adb)
ASM=$(SRC:.c=.asm)
LNK=$(SRC:.c=.lnk)
LST=$(SRC:.c=.lst)
REL=$(SRC:.c=.rel)
RST=$(SRC:.c=.rst)
SYM=$(SRC:.c=.sym)

PROGS=telemetrum.ihx tidongle.ihx
PCDB=$(PROGS:.ihx=.cdb)
PLNK=$(PROGS:.ihx=.lnk)
PMAP=$(PROGS:.ihx=.map)
PMEM=$(PROGS:.ihx=.mem)
PAOM=$(PROGS:.ihx=)

%.rel : %.c $(INC)
	$(CC) -c $(CFLAGS) -o$*.rel $*.c

all: $(PROGS)

telemetrum.ihx: $(TM_REL) Makefile
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(TM_REL)
	sh check-stack ao.h telemetrum.mem

tidongle.ihx: $(TI_REL) Makefile
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(TI_REL)
	sh check-stack ao.h tidongle.mem

tidongle.ihx: telemetrum.ihx

altitude.h: make-altitude
	nickle make-altitude > altitude.h

clean:
	rm -f $(ADB) $(ASM) $(LNK) $(LST) $(REL) $(RST) $(SYM)
	rm -f $(PROGS) $(PCDB) $(PLNK) $(PMAP) $(PMEM) $(PAOM)

install:

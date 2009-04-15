PROG=altos
CC=sdcc
NO_OPT=--nogcse --noinvariant --noinduction --nojtbound --noloopreverse \
	--nolabelopt --nooverlay --peep-asm
DEBUG=--debug

CFLAGS=--model-small $(DEBUG) --less-pedantic

LDFLAGS=--out-fmt-ihx
LDFLAGS_FLASH=$(LDFLAGS) --code-loc 0x0000 --code-size 0x8000 \
	--xram-loc 0xf000 --xram-size 0xf00 --iram-size 0xff

INC = \
	ao.h \
	cc1111.h

SRC = \
	ao_adc.c \
	ao_beep.c \
	ao_cmd.c \
	ao_dma.c \
	ao_ee.c \
	ao_flight.c \
	ao_led.c \
	ao_log.c \
	ao_mutex.c \
	ao_panic.c \
	ao_report.c \
	ao_task.c \
	ao_timer.c \
	ao_usb.c \
	ao_main.c \
	_bp.c
	
ADB=$(SRC:.c=.adb)
ASM=$(SRC:.c=.asm)
LNK=$(SRC:.c=.lnk)
LST=$(SRC:.c=.lst)
REL=$(SRC:.c=.rel)
RST=$(SRC:.c=.rst)
SYM=$(SRC:.c=.sym)

PROGS=$(PROG).ihx
PCDB=$(PROGS:.ihx=.cdb)
PLNK=$(PROGS:.ihx=.lnk)
PMAP=$(PROGS:.ihx=.map)
PMEM=$(PROGS:.ihx=.mem)
PAOM=$(PROGS:.ihx=)

%.rel : %.c $(INC)
	$(CC) -c $(CFLAGS) -o$*.rel $*.c

all: $(PROGS)

$(PROG).ihx: $(REL) Makefile
	$(CC) $(LDFLAGS_FLASH) $(CFLAGS) -o $(PROG).ihx $(REL)
	sh check-stack ao.h $(PROG).mem

clean:
	rm -f $(ADB) $(ASM) $(LNK) $(LST) $(REL) $(RST) $(SYM)
	rm -f $(PROGS) $(PCDB) $(PLNK) $(PMAP) $(PMEM) $(PAOM)

install:

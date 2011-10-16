CC=sdcc
DEBUG=--debug

CFLAGS=--model-small --debug -I../src/core -I../src/cc1111

LDFLAGS=--out-fmt-ihx --code-loc 0xf000 --xram-loc 0xf400 --xram-size 1024 --iram-size 0xff

INC = \
	ao_bringup.h

BRINGUP_SRC = ao_init.c

BRINGUP_REL=$(BRINGUP_SRC:.c=.rel)

XMIT_SRC = \
	ao_radio_init.c \
	ao_radio_xmit.c
XMIT_REL=$(XMIT_SRC:.c=.rel) $(BRINGUP_REL)

LED_SRC = \
	ao_led_blink.c

LED_REL=$(LED_SRC:.c=.rel) $(BRINGUP_REL)

SRC=$(BRINGUP_SRC) $(XMIT_SRC) $(LED_SRC)

ADB=$(SRC:.c=.adb)
ASM=$(SRC:.c=.asm)
LNK=$(SRC:.c=.lnk)
LST=$(SRC:.c=.lst)
REL=$(SRC:.c=.rel)
RST=$(SRC:.c=.rst)
SYM=$(SRC:.c=.sym)

PROGS=ao_radio_xmit.ihx ao_led_blink.ihx

PCDB=$(PROGS:.ihx=.cdb)
PLNK=$(PROGS:.ihx=.lnk)
PMAP=$(PROGS:.ihx=.map)
PMEM=$(PROGS:.ihx=.mem)
PAOM=$(PROGS:.ihx=)

%.rel : %.c
	$(CC) -c $(CFLAGS) -o$*.rel $<

all: $(PROGS)

ao_radio_xmit.ihx: $(XMIT_REL)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(XMIT_REL)

ao_led_blink.ihx: $(LED_REL)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(LED_REL)

clean:
	rm -f $(ADB) $(ASM) $(LNK) $(LST) $(REL) $(RST) $(SYM)
	rm -f $(PROGS) $(PCDB) $(PLNK) $(PMAP) $(PMEM) $(PAOM)

install:

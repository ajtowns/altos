CC=sdcc
NO_OPT=--nogcse --noinvariant --noinduction --nojtbound --noloopreverse \
	--nolabelopt --nooverlay --peep-asm
DEBUG=--debug

CFLAGS=--model-large $(DEBUG) --less-pedantic \
	--no-peep --int-long-reent --float-reent \
	--data-loc 0x30

LDFLAGS=--out-fmt-ihx
LDFLAGS_RAM=$(LDFLAGS) --code-loc 0xf000 --xram-loc 0xf400 --xram-size 1024

LDFLAGS_FLASH=$(LDFLAGS) --code-loc 0x0000 --xram-loc 0xf000 --xram-size 1024

SRC=blink.c
ADB=$(SRC:.c=.adb)
ASM=$(SRC:.c=.asm)
LNK=$(SRC:.c=.lnk)
LST=$(SRC:.c=.lst)
REL=$(SRC:.c=.rel)
RST=$(SRC:.c=.rst)
SYM=$(SRC:.c=.sym)

PROGS=blink-flash.ihx blink-ram.ihx
PCDB=$(PROGS:.ihx=.cdb)
PLNK=$(PROGS:.ihx=.lnk)
PMAP=$(PROGS:.ihx=.map)
PMEM=$(PROGS:.ihx=.mem)
PAOM=$(PROGS:.ihx=)

%.rel : %.c
	$(CC) -c $(CFLAGS) -o$*.rel $<

all: $(PROGS)

blink-ram.ihx: $(REL) Makefile
	$(CC) $(LDFLAGS_RAM) $(CFLAGS) -o blink-ram.ihx $(REL)
	$(CC) $(LDFLAGS_FLASH) $(CFLAGS) -o blink-flash.ihx $(REL)

blink-flash.ihx: blink-ram.ihx

clean:
	rm -f $(ADB) $(ASM) $(LNK) $(LST) $(REL) $(RST) $(SYM)
	rm -f $(PROGS) $(PCDB) $(PLNK) $(PMAP) $(PMEM) $(PAOM)

install:

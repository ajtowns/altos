.NOTPARALLEL: blink-ram blink-flash
KERNEL=/local/src/linux-2.6-aiko-64
KINC=$(KERNEL)/drivers/usb/serial

WARN=-Wall -Wpointer-arith -Wstrict-prototypes -Wmissing-prototypes\
	-Wmissing-declarations -Wnested-externs -fno-strict-aliasing 
CFLAGS=-g -I$(KINC) $(WARN)
LIBS=-lusb

KERNEL_OBJS=cccp.o
LIBUSB_OBJS=cp-usb.o

SRCS=ccdbg.c ccdbg-command.c ccdbg-debug.c ccdbg-flash.c \
	ccdbg-hex.c ccdbg-io.c ccdbg-memory.c \
	$(LIBUSB_OBJS)

OBJS=$(SRCS:.c=.o)

INCS=ccdbg.h cccp.h

PROG=ccdbg

LOAD=blinks

all: $(PROG) $(LOAD)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f $(PROG) $(OBJS)
	+make -f Makefile.blink clean

$(OBJS): $(INCS)

blinks: blink.c Makefile.blink
	+make -f Makefile.blink

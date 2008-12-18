KERNEL=/local/src/linux-2.6-aiko-64
KINC=$(KERNEL)/drivers/usb/serial

WARN=-Wall -Wpointer-arith -Wstrict-prototypes -Wmissing-prototypes\
	-Wmissing-declarations -Wnested-externs -fno-strict-aliasing 
CFLAGS=-g -I$(KINC) $(WARN)
LIBS=-lusb

KERNEL_OBJS=cccp.o
LIBUSB_OBJS=cp-usb.o

OBJS=ccdbg.o ccdbg-command.o ccdbg-debug.o \
	ccdbg-io.o ccdbg-memory.o \
	$(LIBUSB_OBJS)
INCS=ccdbg.h cccp.h

PROG=ccdbg

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f $(PROG) $(OBJS)

$(OBJS): $(INCS)

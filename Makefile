KERNEL=/local/src/linux-2.6-aiko-64
KINC=$(KERNEL)/drivers/usb/serial

WARN=-Wall -Wpointer-arith -Wstrict-prototypes -Wmissing-prototypes\
	-Wmissing-declarations -Wnested-externs -fno-strict-aliasing 
CFLAGS=-g -I$(KINC) $(WARN)

OBJS=ccdbg.o ccdbg-command.o ccdbg-io.o cccp.o
INCS=ccdbg.h cccp.h

PROG=ccdbg

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -f $(PROG) $(OBJS)

$(OBJS): $(INCS)

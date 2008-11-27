KERNEL=/local/src/linux-2.6-aiko-64
KINC=$(KERNEL)/drivers/usb/serial

CFLAGS=-g -I$(KINC)

OBJS=ccdbg-command.o ccdbg-io.o cccp.o
INCS=ccdbg.h cccp.h

PROG=ccdbg

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -f $(PROG) $(OBJS)

$(OBJS): $(INCS)

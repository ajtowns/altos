#
# AltOS build
#
#

include ../lpc/Makefile.defs

INC = \
	ao.h \
	ao_arch.h \
	ao_arch_funcs.h \
	ao_pins.h \
	ao_product.h \
	lpc.h

#
# Common AltOS sources
#
ALTOS_SRC = \
	ao_interrupt.c \
	ao_boot_chain.c \
	ao_romconfig.c \
	ao_product.c \
	ao_mutex.c \
	ao_panic.c \
	ao_stdio.c \
	ao_task.c \
	ao_cmd.c \
	ao_timer_lpc.c \
	ao_exti_lpc.c \
	ao_usb_lpc.c \
	ao_serial_lpc.c \
	ao_spi_lpc.c \
	ao_led_lpc.c

PRODUCT=usbtrng-v0.1
PRODUCT_DEF=-DUSBTRNG_V_0_1
IDPRODUCT=0x0028

CFLAGS = $(PRODUCT_DEF) $(LPC_CFLAGS) -g -Os

PROGNAME=usbtrng-v0.1
PROG=$(PROGNAME)-$(VERSION).elf
HEX=$(PROGNAME)-$(VERSION).ihx

SRC=$(ALTOS_SRC) ao_usbtrng.c
OBJ=$(SRC:.c=.o)

all: $(PROG) $(HEX)

$(PROG): Makefile $(OBJ) altos.ld
	$(call quiet,CC) $(LDFLAGS) $(CFLAGS) -o $(PROG) $(OBJ) $(LIBS)

ao_product.h: ao-make-product.5c ../Version
	$(call quiet,NICKLE,$<) $< -m altusmetrum.org -i $(IDPRODUCT) -p $(PRODUCT) -v $(VERSION) > $@

$(OBJ): $(INC)

load: $(PROG)
	lpc-load $(PROG)

distclean:	clean

clean:
	rm -f *.o $(PROGNAME)-*.elf $(PROGNAME)-*.ihx
	rm -f ao_product.h

install:

uninstall:

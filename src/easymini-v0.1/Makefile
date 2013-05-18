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
	ao_romconfig.c \
	ao_product.c \
	ao_mutex.c \
	ao_panic.c \
	ao_stdio.c \
	ao_storage.c \
	ao_report.c \
	ao_ignite.c \
	ao_flight.c \
	ao_kalman.c \
	ao_sample.c \
	ao_data.c \
	ao_convert_pa.c \
	ao_led_lpc.c \
	ao_task.c \
	ao_log.c \
	ao_log_tiny.c \
	ao_cmd.c \
	ao_config.c \
	ao_timer_lpc.c \
	ao_exti_lpc.c \
	ao_serial_lpc.c \
	ao_usb_lpc.c \
	ao_spi_lpc.c \
	ao_adc_lpc.c \
	ao_beep_lpc.c \
	ao_m25.c \
	ao_ms5607.c

PRODUCT=EasyMini-v0.1
PRODUCT_DEF=-DEASYMINI_V_0_1
IDPRODUCT=0x000a

CFLAGS = $(PRODUCT_DEF) $(LPC_CFLAGS) -g -Os

PROGNAME=easymini-v0.1
PROG=$(PROGNAME)-$(VERSION).elf

SRC=$(ALTOS_SRC) ao_easymini.c
OBJ=$(SRC:.c=.o)

all: $(PROG)

LDFLAGS=-L../lpc -Wl,-Taltos.ld

$(PROG): Makefile $(OBJ)
	$(call quiet,CC) $(LDFLAGS) $(CFLAGS) -o $(PROG) $(OBJ) $(SAT_CLIB) -lgcc

ao_product.h: ao-make-product.5c ../Version
	$(call quiet,NICKLE,$<) $< -m altusmetrum.org -i $(IDPRODUCT) -p $(PRODUCT) -v $(VERSION) > $@

$(OBJ): $(INC)

load: $(PROG)
	lpc-load $(PROG)

distclean:	clean

clean:
	rm -f *.o $(PROG)
	rm -f ao_product.h

install:

uninstall:

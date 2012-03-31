#
# AltOS build
#
#

include ../stm/Makefile.defs

INC = \
	ao.h \
	ao_arch.h \
	ao_pins.h \
	ao_product.h

#
# Common AltOS sources
#
ALTOS_SRC = \
	ao_interrupt.c \
	ao_product.c \
	ao_romconfig.c \
	ao_cmd.c \
	ao_task.c \
	ao_led.c \
	ao_stdio.c \
	ao_panic.c \
	ao_timer.c \
	ao_serial_stm.c \
	ao_lcd_stm.c \
	ao_lcd_font.c

PRODUCT=StmDemo-v0.0
PRODUCT_DEF=-DSTM_DEMO
IDPRODUCT=0x000a

CFLAGS = $(PRODUCT_DEF) $(STM_CFLAGS) -g -Os

PROG=stm-demo

SRC=$(ALTOS_SRC) ao_demo.c
OBJ=$(SRC:.c=.o)

all: $(PROG)

LDFLAGS=-L../stm -Wl,-Taltos.ld

$(PROG): Makefile $(OBJ)
	$(call quiet,CC) $(LDFLAGS) $(CFLAGS) -o $(PROG) $(OBJ) $(SAT_CLIB) -lgcc

ao_product.h: ao-make-product.5c ../Version
	$(call quiet,NICKLE,$<) $< -m altusmetrum.org -i $(IDPRODUCT) -p $(PRODUCT) -v $(VERSION) > $@

$(OBJ): $(INC)

distclean:	clean

clean:
	rm -f $(OBJ)
	rm -f ao_product.h

install:

uninstall:
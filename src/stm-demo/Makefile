#
# AltOS build
#
#
vpath % ..:../core:../product:../drivers:../stm
vpath make-altitude ../util
vpath make-kalman ../util
vpath kalman.5c ../kalman
vpath kalman_filter.5c ../kalman
vpath load_csv.5c ../kalman
vpath matrix.5c ../kalman
vpath ao-make-product.5c ../util

#PROGRAMMER=stk500v2 -P usb
#PROGRAMMER=usbtiny
#LOADCMD=avrdude
#LOADARG=-p $(DUDECPUTYPE) -c $(PROGRAMMER) -e -U flash:w:
CC=arm-none-eabi-gcc
#OBJCOPY=avr-objcopy

ifndef VERSION
include ../Version
endif

INC = \
	ao.h \
	ao_arch.h \
	ao_pins.h \
	altitude.h \
	ao_kalman.h

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
	ao_serial_stm.c

PRODUCT=StmDemo-v0.0
PRODUCT_DEF=-DSTM_DEMO
IDPRODUCT=0x000a
CPU=cortex-m3
CFLAGS = $(PRODUCT_DEF) -I. -I../stm -I../core -I..
CFLAGS += -g -std=gnu99 -Os -mlittle-endian -mcpu=cortex-m3 -mthumb -ffreestanding -nostdlib -I../stm $(CINC)

NICKLE=nickle

PROG=stm-demo

SRC=$(ALTOS_SRC) ao_demo.c
OBJ=$(SRC:.c=.o)

V=0
# The user has explicitly enabled quiet compilation.
ifeq ($(V),0)
quiet = @printf "  $1 $2 $@\n"; $($1)
endif
# Otherwise, print the full command line.
quiet ?= $($1)

all: $(PROG)

CLIB=/home/keithp/sat/lib/pdclib.a
CINC=-I/home/keithp/sat/include
LDFLAGS=-L../stm -Wl,-Taltos.ld

$(PROG): Makefile $(OBJ)
	$(call quiet,CC) $(LDFLAGS) $(CFLAGS) -o $(PROG) $(OBJ) $(CLIB) -lgcc

../altitude.h: make-altitude
	nickle $< > $@

$(OBJ): $(INC)

ao_product.h: ao-make-product.5c ../Version
	$(call quiet,NICKLE,$<) $< -m altusmetrum.org -i $(IDPRODUCT) -p $(PRODUCT) -v $(VERSION) > $@

ao_product.rel: ao_product.c ao_product.h
	$(call quiet,CC) -c $(CFLAGS) -D PRODUCT_DEFS='\"ao_product.h\"' -o$@ $<

distclean:	clean

clean:
	rm -f $(OBJ)
	rm -f ao_product.h

install:

uninstall:

$(OBJ): ao.h ao_product.h
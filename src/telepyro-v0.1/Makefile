#
# AltOS build
#
#
vpath % .:..:../core:../product:../drivers:../avr
vpath ao-make-product.5c ../util

MCU=atmega32u4
DUDECPUTYPE=m32u4
#PROGRAMMER=stk500v2 -P usb
PROGRAMMER=usbtiny
LOADCMD=avrdude
LOADARG=-p $(DUDECPUTYPE) -c $(PROGRAMMER) -e -U flash:w:
CC=avr-gcc
OBJCOPY=avr-objcopy

ifndef VERSION
include ../Version
endif

INC = \
	ao.h \
	ao_usb.h \
	ao_pins.h \
	ao_arch.h \
	ao_arch_funcs.h \
	ao_product.h

ALTOS_SRC = \
	ao_clock.c \
	ao_cmd.c \
	ao_mutex.c \
	ao_panic.c \
	ao_product.c \
	ao_stdio.c \
	ao_task.c \
	ao_timer.c \
	ao_led.c \
	ao_avr_stdio.c \
	ao_romconfig.c \
	ao_usb_avr.c \
	ao_adc_avr.c \
	ao_pyro_slave.c \
	ao_spi_slave.c \
	ao_eeprom_avr.c \
	ao_storage.c \
	ao_config.c \
	ao_pyro.c

PRODUCT=TelePyro-v0.1
MCU=atmega32u4
PRODUCT_DEF=-DTELEPYRO
IDPRODUCT=0x0011
CFLAGS = $(PRODUCT_DEF) -I. -I../avr -I../core -I..
CFLAGS += -mmcu=$(MCU) -Wall -Wstrict-prototypes -O3 -mcall-prologues -DAVR

NICKLE=nickle

PROG=telepyro-v0.1

SRC=$(ALTOS_SRC) ao_telepyro.c
OBJ=$(SRC:.c=.o)

V=0
# The user has explicitly enabled quiet compilation.
ifeq ($(V),0)
quiet = @printf "  $1 $2 $@\n"; $($1)
endif
# Otherwise, print the full command line.
quiet ?= $($1)

all: $(PROG)

CHECK=sh ../util/check-avr-mem

$(PROG): Makefile $(OBJ)
	$(call quiet,CC) $(LDFLAGS) $(CFLAGS) -o $(PROG) $(OBJ)
	$(call quiet,CHECK) $(PROG) || ($(RM) -f $(PROG); exit 1)

$(PROG).hex: $(PROG)
	avr-size $(PROG)
	$(OBJCOPY) -R .eeprom -O ihex $(PROG) $@


load: $(PROG).hex
	$(LOADCMD) $(LOADARG)$(PROG).hex

ao_product.h: ao-make-product.5c ../Version
	$(call quiet,NICKLE,$<) $< -m altusmetrum.org -i $(IDPRODUCT) -p $(PRODUCT) -v $(VERSION) > $@

ao_product.o: ao_product.c ao_product.h

%.o : %.c
	$(call quiet,CC) -c $(CFLAGS) $<

distclean:	clean

clean:
	rm -f $(OBJ)
	rm -f ao_product.h

install:

uninstall:

$(OBJ): $(INC)

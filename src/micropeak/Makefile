#
# Tiny AltOS build
#
#
vpath % ../attiny:../drivers:../core:..
vpath ao-make-product.5c ../util
vpath make-altitude-pa ../util

MCU=attiny85
DUDECPUTYPE=t85
#PROGRAMMER=stk500v2 -P usb
PROGRAMMER=usbtiny
LOADCMD=avrdude
LOADSLOW=-i 32 -B 32
LOADARG=-p $(DUDECPUTYPE) -c $(PROGRAMMER) -e -U flash:w:
CC=avr-gcc
OBJCOPY=avr-objcopy

ifndef VERSION
include ../Version
endif

# Support for a logging EEPROM
#
#EEPROM_SRC=ao_async.c \
#	ao_i2c_attiny.c \
#	ao_at24c.c
#

ALTOS_SRC = \
	ao_micropeak.c \
	ao_spi_attiny.c \
	ao_led.c \
	ao_clock.c \
	ao_ms5607.c \
	ao_exti.c \
	ao_convert_pa.c \
	ao_report_tiny.c \
	ao_notask.c \
	ao_eeprom_tiny.c \
	ao_panic.c \
	$(EEPROM_SRC)

INC=\
	ao.h \
	ao_pins.h \
	ao_arch.h \
	ao_arch_funcs.h \
	ao_exti.h \
	ao_ms5607.h \
	altitude-pa.h

IDPRODUCT=0
PRODUCT=MicroPeak-v0.1
PRODUCT_DEF=-DMICROPEAK
CFLAGS = $(PRODUCT_DEF) -I. -I../attiny -I../core -I.. -I../drivers
CFLAGS += -g -mmcu=$(MCU) -Wall -Wstrict-prototypes -O3 -mcall-prologues -DATTINY

NICKLE=nickle

PROG=micropeak-v0.1

SRC=$(ALTOS_SRC)
OBJ=$(SRC:.c=.o)

V=0
# The user has explicitly enabled quiet compilation.
ifeq ($(V),0)
quiet = @printf "  $1 $2 $@\n"; $($1)
endif
# Otherwise, print the full command line.
quiet ?= $($1)

all: $(PROG) $(PROG).hex

CHECK=sh ../util/check-avr-mem

$(PROG): Makefile $(OBJ)
	$(call quiet,CC) $(LDFLAGS) $(CFLAGS) -o $(PROG) $(OBJ)
	$(call quiet,CHECK) $(PROG) || ($(RM) -f $(PROG); exit 1)

$(PROG).hex: $(PROG)
	avr-size $(PROG)
	$(OBJCOPY) -R .eeprom -O ihex $(PROG) $@


load: $(PROG).hex
	$(LOADCMD) $(LOADARG)$(PROG).hex

load-slow: $(PROG).hex
	$(LOADCMD) $(LOADSLOW) $(LOADARG)$(PROG).hex

ao_product.h: ao-make-product.5c ../Version
	$(call quiet,NICKLE,$<) $< -m altusmetrum.org -i $(IDPRODUCT) -p $(PRODUCT) -v $(VERSION) > $@

ao_product.o: ao_product.c ao_product.h

%.o : %.c $(INC)
	$(call quiet,CC) -c $(CFLAGS) $<

distclean:	clean

clean:
	rm -f *.o $(PROG) $(PROG).hex
	rm -f ao_product.h

../altitude-pa.h: make-altitude-pa
	nickle $< > $@

install:

uninstall:

$(OBJ): ao_product.h $(INC)

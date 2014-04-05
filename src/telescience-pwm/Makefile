#
# AltOS build
#
#
vpath % ..:../kernel:../product:../drivers:../avr
vpath ao-make-product.5c ../util

include ../avr/Makefile.defs

MCU=atmega32u4
DUDECPUTYPE=m32u4
#PROGRAMMER=stk500v2 -P usb
LOADARG=-p $(DUDECPUTYPE) -c $(PROGRAMMER) -e -U flash:w:

#LDFLAGS=-L$(LDSCRIPTS) -Tavr5.x

INC = \
	ao.h \
	ao_arch.h \
	ao_usb.h \
	ao_pins.h \
	ao_product.h

#
# Common AltOS sources
#
TELESCIENCE_STORAGE= \
	ao_m25.c \
	ao_spi_usart.c \
	ao_storage.c

TELESCIENCE_LOG= \
	ao_log_single.c \
	ao_log_telescience.c

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
	ao_science_slave.c \
	ao_spi_slave.c \
	ao_pwmin.c \
	$(TELESCIENCE_STORAGE)\
	$(TELESCIENCE_LOG)

PRODUCT=TeleScience-PWM
MCU=atmega32u4
PRODUCT_DEF=-DTELESCIENCE -DTELESCIENCE_PWM
IDPRODUCT=0x0011
CFLAGS = $(PRODUCT_DEF) -I. -I../avr -I../kernel -I..
CFLAGS += -g -mmcu=$(MCU) -Wall -Wstrict-prototypes -O3 -mcall-prologues -DAVR

NICKLE=nickle

PROG=telescience-pwm

SRC=$(ALTOS_SRC) ao_telescience.c
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

%.o : %.c $(INC)
	$(call quiet,CC) -c $(CFLAGS) $<

distclean:	clean

clean:
	rm -f *.o $(PROG) $(PROG).hex
	rm -f ao_product.h

install:

uninstall:

$(OBJ): ao_product.h $(INC)

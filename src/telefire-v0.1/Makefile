#
# TeleFire build file
#

TELEFIRE_VER=0.1
TELEFIRE_DEF=0_1

vpath %.c ..:../kernel:../cc1111:../drivers:../product
vpath %.h ..:../kernel:../cc1111:../drivers:../product
vpath ao-make-product.5c ../util

ifndef VERSION
include ../Version
endif

INC = \
	ao.h \
	ao_pins.h \
	ao_arch.h \
	ao_arch_funcs.h \
	ao_pad.h \
	cc1111.h \
	ao_product.h

CORE_SRC = \
	ao_cmd.c \
	ao_config.c \
	ao_convert.c \
	ao_mutex.c \
	ao_panic.c \
	ao_stdio.c \
	ao_storage.c \
	ao_task.c \
	ao_freq.c

CC1111_SRC = \
	ao_adc.c \
	ao_aes.c \
	ao_beep.c \
	ao_dma.c \
	ao_intflash.c \
	ao_radio.c \
	ao_radio_cmac.c \
	ao_romconfig.c \
	ao_serial.c \
	ao_spi.c \
	ao_string.c \
	ao_timer.c \
	ao_usb.c \
	_bp.c

DRIVER_SRC = \
	ao_pca9922.c \
	ao_74hc497.c \
	ao_pad.c \
	ao_radio_cmac_cmd.c

PRODUCT_SRC = \
	ao_telefire.c

SRC = \
	$(CORE_SRC) \
	$(CC1111_SRC) \
	$(DRIVER_SRC) \
	$(PRODUCT_SRC)

PROGNAME = telefire-v$(TELEFIRE_VER)
PROG = $(PROGNAME)-$(VERSION).ihx
PRODUCT=TeleFire-v$(TELEFIRE_VER)
PRODUCT_DEF=-DTELEFIRE_V_$(TELEFIRE_DEF)
IDPRODUCT=0x000f
CODESIZE=0x6700

include ../cc1111/Makefile.cc1111

NICKLE=nickle
CHECK_STACK=sh ../util/check-stack

V=0
# The user has explicitly enabled quiet compilation.
ifeq ($(V),0)
quiet = @printf "  $1 $2 $@\n"; $($1)
endif
# Otherwise, print the full command line.
quiet ?= $($1)

all: $(PROG)

$(PROG): $(REL) Makefile
	$(call quiet,CC) $(LDFLAGS) $(CFLAGS) -o $(PROG) $(REL)
	$(call quiet,CHECK_STACK) ../cc1111/ao_arch.h $(PMEM)  || rm $@

ao_product.h: ao-make-product.5c ../Version
	$(call quiet,NICKLE,$<) $< -m altusmetrum.org -i $(IDPRODUCT) -p $(PRODUCT) -v $(VERSION) > $@

distclean:	clean

clean: clean-cc1111

install:

uninstall:


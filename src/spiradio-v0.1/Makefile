#
# SpiRadio build file
#

SPIRADIO_VER=0.1
SPIRADIO_DEF=0_1

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
	cc1111.h \
	ao_product.h \
	ao_radio_spi.h

CORE_SRC = \
	ao_cmd.c \
	ao_config.c \
	ao_mutex.c \
	ao_panic.c \
	ao_stdio.c \
	ao_task.c \
	ao_freq.c

CC1111_SRC = \
	ao_aes.c \
	ao_dma.c \
	ao_led.c \
	ao_radio.c \
	ao_radio_cmac.c \
	ao_radio_slave.c \
	ao_romconfig.c \
	ao_serial.c \
	ao_spi.c \
	ao_string.c \
	ao_timer.c \
	_bp.c

PRODUCT_SRC = \
	ao_spiradio.c

SRC = \
	$(CORE_SRC) \
	$(CC1111_SRC) \
	$(PRODUCT_SRC)

PROGNAME = spiradio-v$(SPIRADIO_VER)
PROG = $(PROGNAME)-$(VERSION).ihx
PRODUCT=SpiRadio-v$(SPIRADIO_VER)
PRODUCT_DEF=-DSPIRADIO_V_$(SPIRADIO_DEF)
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


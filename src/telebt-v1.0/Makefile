#
# TeleBT build file
#

TELEBT_VER=1.0
TELEBT_DEF=1_0

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
	ao_product.h

CORE_SRC = \
	ao_cmd.c \
	ao_config.c \
	ao_gps_print.c \
	ao_monitor.c \
	ao_mutex.c \
	ao_panic.c \
	ao_state.c \
	ao_stdio.c \
	ao_task.c \
	ao_freq.c

CC1111_SRC = \
	ao_dbg.c \
	ao_adc.c \
	ao_dma.c \
	ao_led.c \
	ao_packet.c \
	ao_packet_master.c \
	ao_radio.c \
	ao_romconfig.c \
	ao_serial.c \
	ao_string.c \
	ao_timer.c \
	ao_usb.c \
	_bp.c

DRIVER_SRC = \
	ao_btm.c

PRODUCT_SRC = \
	ao_telebt.c

SRC = \
	$(CORE_SRC) \
	$(CC1111_SRC) \
	$(DRIVER_SRC) \
	$(PRODUCT_SRC)

PROGNAME = telebt-v$(TELEBT_VER)
PROG = $(PROGNAME)-$(VERSION).ihx
PRODUCT=TeleBT-v$(TELEBT_VER)
PRODUCT_DEF=-DTELEBT_V_$(TELEBT_DEF)
IDPRODUCT=0x000e

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
	$(call quiet,CHECK_STACK) ../cc1111/ao_arch.h $(PMEM) || rm $@

ao_product.h: ao-make-product.5c ../Version
	$(call quiet,NICKLE,$<) $< -m altusmetrum.org -i $(IDPRODUCT) -p $(PRODUCT) -v $(VERSION) > $@

distclean:	clean

clean: clean-cc1111

install:

uninstall:

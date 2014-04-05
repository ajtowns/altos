#
# TeleShield build file
#
# The various telemetrum versions differ only
# in which flash and GPS drivers are included,
# so the per-board makefiles simply define
# TM_VER, TM_DEF, TM_INC and TM_SRC and include
# this file

TELESHIELD_VER=0.1
TELESHIELD_DEF=0_1

TELESHIELD_INC =

TELESHIELD_SRC = \
	ao_beep.c \
	ao_btm.c \
	ao_spi.c

vpath %.c ..:../kernel:../cc1111:../drivers:../product:.
vpath %.h ..:../kernel:../cc1111:../drivers:../product:.
vpath ao-make-product.5c ../util

ifndef VERSION
include ../Version
endif

INC = \
	ao.h \
	ao_pins.h \
	cc1111.h \
	ao_product.h \
	$(TELESHIELD_INC)

CORE_SRC = \
	ao_cmd.c \
	ao_config.c \
	ao_monitor.c \
	ao_mutex.c \
	ao_panic.c \
	ao_state.c \
	ao_storage.c \
	ao_stdio.c \
	ao_task.c \
	ao_freq.c

CC1111_SRC = \
	ao_dbg.c \
	ao_dma.c \
	ao_led.c \
	ao_intflash.c \
	ao_packet.c \
	ao_packet_slave.c \
	ao_radio.c \
	ao_romconfig.c \
	ao_serial.c \
	ao_string.c \
	ao_timer.c \
	ao_usb.c \
	_bp.c

DRIVER_SRC = \
	$(TELESHIELD_SRC)

PRODUCT_SRC = \
	ao_teleshield.c \
	ao_ardu_serial.c

SRC = \
	$(CORE_SRC) \
	$(CC1111_SRC) \
	$(DRIVER_SRC) \
	$(PRODUCT_SRC)

PROGNAME = teleshield-v$(TELESHIELD_VER)
PROG = $(PROGNAME)-$(VERSION).ihx
PRODUCT=TeleShield-v$(TELESHIELD_VER)
PRODUCT_DEF=-DTELESHIELD_V_$(TELESHIELD_DEF)
IDPRODUCT=0x0013
CODESIZE=0x7800

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


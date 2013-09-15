#
# TeleTerra build file
#

vpath %.c ..:../core:../cc1111:../drivers:../product
vpath %.h ..:../core:../cc1111:../drivers:../product
vpath ao-make-product.5c ../util

ifndef VERSION
include ../Version
endif

INC = \
	ao.h \
	ao_pins.h \
	cc1111.h \
	ao_product.h

CORE_SRC = \
	ao_cmd.c \
	ao_config.c \
	ao_monitor.c \
	ao_log_single.c \
	ao_log_telem.c \
	ao_mutex.c \
	ao_panic.c \
	ao_report.c \
	ao_sqrt.c \
	ao_stdio.c \
	ao_storage.c \
	ao_task.c \
	ao_freq.c

CC1111_SRC = \
	ao_battery.c \
	ao_beep.c \
	ao_button.c \
	ao_dbg.c \
	ao_dma.c \
	ao_packet.c \
	ao_packet_master.c \
	ao_radio.c \
	ao_romconfig.c \
	ao_serial.c \
	ao_spi.c \
	ao_string.c \
	ao_timer.c \
	ao_usb.c \
	ao_lcd_port.c \
	_bp.c

DRIVER_SRC = \
	ao_m25.c \
	ao_lcd.c \
	ao_gps_skytraq.c \
	ao_gps_show.c

PRODUCT_SRC = \
	ao_teleterra_0_2.c \
	ao_terraui.c

SRC = \
	$(CORE_SRC) \
	$(CC1111_SRC) \
	$(DRIVER_SRC) \
	$(PRODUCT_SRC)

TELETERRA_VER=0.2
TELETERRA_DEF=0_2
PROGNAME = teleterra-v$(TELETERRA_VER)
PROG = $(PROGNAME)-$(VERSION).ihx
PRODUCT=TeleTerra-v$(TELETERRA_VER)
PRODUCT_DEF=-DTELETERRA_V_$(TELETERRA_DEF)
IDPRODUCT=0x000d

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


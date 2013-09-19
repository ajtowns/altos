#
# TeleMini build file
#

TELEMINI_VER=2.0
TELEMINI_DEF=2_0

vpath %.c ..:../core:../cc1111:../drivers:../product
vpath %.h ..:../core:../cc1111:../drivers:../product
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
	ao_ms5607.h \
	ao_ms5607_convert_8051.c \
	ao_product.h \
	ao_int64.h \
	ao_sample.h \
	ao_exti.h \
	ao_task.h

CORE_SRC = \
	ao_cmd.c \
	ao_config.c \
	ao_convert.c \
	ao_flight.c \
	ao_kalman.c \
	ao_log.c \
	ao_log_mini.c \
	ao_mutex.c \
	ao_panic.c \
	ao_report.c \
	ao_sample.c \
	ao_stdio.c \
	ao_storage.c \
	ao_task.c \
	ao_telemetry.c \
	ao_freq.c \
	ao_int64.c

CC1111_SRC = \
	ao_adc.c \
	ao_dma.c \
	ao_ignite.c \
	ao_led.c \
	ao_packet.c \
	ao_packet_slave.c \
	ao_radio.c \
	ao_romconfig.c \
	ao_string.c \
	ao_spi.c \
	ao_usb.c \
	ao_convert_pa.c \
	ao_beep.c \
	ao_timer.c \
	ao_exti.c \
	_bp.c

DRIVER_SRC = \
	ao_ms5607.c \
	ao_m25.c

PRODUCT_SRC = \
	ao_telemini.c

SRC = \
	$(CORE_SRC) \
	$(CC1111_SRC) \
	$(DRIVER_SRC) \
	$(PRODUCT_SRC)

PROGNAME = telemini-v$(TELEMINI_VER)
PROG = $(PROGNAME)-$(VERSION).ihx
PRODUCT=TeleMini-v$(TELEMINI_VER)
PRODUCT_DEF=-DTELEMINI_V_$(TELEMINI_DEF)
IDPRODUCT=0x0027

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


#
# AltOS build
#
#

include ../stm/Makefile.defs

INC = \
	ao.h \
	ao_arch.h \
	ao_arch_funcs.h \
	ao_pins.h \
	altitude.h \
	ao_kalman.h \
	ao_product.h \
	ao_ms5607.h \
	ao_hmc5883.h \
	ao_mpu6000.h \
	stm32l.h

#
# Common AltOS sources
#
ALTOS_SRC = \
	ao_interrupt.c \
	ao_product.c \
	ao_romconfig.c \
	ao_cmd.c \
	ao_config.c \
	ao_task.c \
	ao_led.c \
	ao_stdio.c \
	ao_panic.c \
	ao_timer.c \
	ao_mutex.c \
	ao_serial_stm.c \
	ao_gps_skytraq.c \
	ao_cc1120.c \
	ao_freq.c \
	ao_dma_stm.c \
	ao_spi_stm.c \
	ao_ms5607.c \
	ao_adc_stm.c \
	ao_beep_stm.c \
	ao_storage.c \
	ao_m25.c \
	ao_usb_stm.c \
	ao_exti_stm.c \
	ao_packet.c \
	ao_packet_slave.c \
	ao_i2c_stm.c \
	ao_hmc5883.c \
	ao_mpu6000.c

PRODUCT=MegaMetrum-v0.1
PRODUCT_DEF=-DMEGAMETRUM
IDPRODUCT=0x000a

CFLAGS = $(PRODUCT_DEF) $(STM_CFLAGS) -O0 -g

PROG=megametrum-v0.1-$(VERSION).elf

SRC=$(ALTOS_SRC) ao_megametrum.c
OBJ=$(SRC:.c=.o)

all: $(PROG)

$(PROG): Makefile $(OBJ)
	$(call quiet,CC) $(LDFLAGS) $(CFLAGS) -o $(PROG) $(OBJ) $(SAT_CLIB) -lgcc

../altitude.h: make-altitude
	nickle $< > $@

$(OBJ): $(INC)

ao_product.h: ao-make-product.5c ../Version
	$(call quiet,NICKLE,$<) $< -m altusmetrum.org -i $(IDPRODUCT) -p $(PRODUCT) -v $(VERSION) > $@

distclean:	clean

clean:
	rm -f $(OBJ)
	rm -f ao_product.h

install:

uninstall:

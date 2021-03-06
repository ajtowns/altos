#
# AltOS build
#
#

include ../stm/Makefile.defs

INC = \
	ao.h \
	ao_arch.h \
	ao_arch_funcs.h \
	ao_boot.h \
	ao_companion.h \
	ao_data.h \
	ao_sample.h \
	ao_pins.h \
	altitude-pa.h \
	ao_kalman.h \
	ao_product.h \
	ao_ms5607.h \
	ao_hmc5883.h \
	ao_mpu6000.h \
	ao_mma655x.h \
	ao_profile.h \
	ao_task.h \
	ao_whiten.h \
	ao_sample_profile.h \
	ao_quaternion.h \
	math.h \
	ao_mpu.h \
	stm32l.h \
	math.h \
	Makefile

#
# Common AltOS sources
#
#	ao_hmc5883.c

#PROFILE=ao_profile.c
#PROFILE_DEF=-DAO_PROFILE=1

#SAMPLE_PROFILE=ao_sample_profile.c \
#	ao_sample_profile_timer.c
#SAMPLE_PROFILE_DEF=-DHAS_SAMPLE_PROFILE=1

#STACK_GUARD=ao_mpu_stm.c
#STACK_GUARD_DEF=-DHAS_STACK_GUARD=1

MATH_SRC=\
	ef_acos.c \
	ef_sqrt.c \
	ef_rem_pio2.c \
	kf_cos.c \
	kf_sin.c \
	kf_rem_pio2.c \
	sf_copysign.c \
	sf_cos.c \
	sf_fabs.c \
	sf_floor.c \
	sf_scalbn.c \
	sf_sin.c \
	ef_log.c

ALTOS_SRC = \
	ao_boot_chain.c \
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
	ao_ignite.c \
	ao_freq.c \
	ao_dma_stm.c \
	ao_spi_stm.c \
	ao_data.c \
	ao_ms5607.c \
	ao_mma655x.c \
	ao_hmc5883.c \
	ao_adc_stm.c \
	ao_beep_stm.c \
	ao_eeprom_stm.c \
	ao_storage.c \
	ao_m25.c \
	ao_usb_stm.c \
	ao_exti_stm.c \
	ao_report.c \
	ao_i2c_stm.c \
	ao_mpu6000.c \
	ao_convert_pa.c \
	ao_convert_volt.c \
	ao_log.c \
	ao_log_mega.c \
	ao_sample.c \
	ao_kalman.c \
	ao_flight.c \
	ao_companion.c \
	ao_pyro.c \
	$(MATH_SRC) \
	$(PROFILE) \
	$(SAMPLE_PROFILE) \
	$(STACK_GUARD)

PRODUCT=EasyMega-v0.1
PRODUCT_DEF=-DEASYMEGA
IDPRODUCT=0x0023

CFLAGS = $(PRODUCT_DEF) $(STM_CFLAGS) $(PROFILE_DEF) $(SAMPLE_PROFILE_DEF) $(STACK_GUARD_DEF) -Os -g

PROGNAME=easymega-v0.1
PROG=$(PROGNAME)-$(VERSION).elf
HEX=$(PROGNAME)-$(VERSION).ihx

SRC=$(ALTOS_SRC) ao_easymega.c
OBJ=$(SRC:.c=.o)

all: $(PROG) $(HEX)

$(PROG): Makefile $(OBJ) altos.ld
	$(call quiet,CC) $(LDFLAGS) $(CFLAGS) -o $(PROG) $(OBJ) $(LIBS)

../altitude-pa.h: make-altitude-pa
	nickle $< > $@

$(OBJ): $(INC)

ao_product.h: ao-make-product.5c ../Version
	$(call quiet,NICKLE,$<) $< -m altusmetrum.org -i $(IDPRODUCT) -p $(PRODUCT) -v $(VERSION) > $@

distclean:	clean

clean:
	rm -f *.o $(PROGNAME)-*.elf $(PROGNAME)-*.ihx
	rm -f ao_product.h

install:

uninstall:

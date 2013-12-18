vpath % ..:../core:../product:../drivers:../stm
vpath ao-make-product.5c ../util

ifndef VERSION
include ../Version
endif
TOPDIR=..
include $(TOPDIR)/Makedefs

CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

C_LIB=$(PDCLIB_LIBS_M3)

DEF_CFLAGS=-g -std=gnu99 -Os -mlittle-endian -mthumb -ffreestanding -nostdlib -I. -I../stm $(PDCLIB_INCLUDES)

# to run from SRAM
LD_FLAGS_RAM=-L../stm -Wl,-Taltos-ram.ld
LD_FLAGS=-L../stm -Wl,-Tbringup.ld

CFLAGS=$(DEF_CFLAGS) -mcpu=cortex-m3 -DCONFIG_STM32L_DISCOVERY

SRC=bringup.c ao_interrupt.c
OBJ=$(SRC:.c=.o)

all: bringup-ram.elf bringup.elf

%.bin: %.elf
	$(OBJCOPY) -O binary $^ $@

bringup.elf: $(OBJ) bringup.ld
	$(CC) $(CFLAGS) $(LD_FLAGS) -o $@ $(OBJ) $(C_LIB) -lgcc

bringup-ram.elf: $(OBJ) altos-ram.ld
	$(CC) $(CFLAGS) $(LD_FLAGS_RAM) -o $@ $(OBJ) $(C_LIB) -lgcc

clean:
	rm -f *.o
	rm -rf *.elf
	rm -rf *.bin

.PHONY: all clean

install:

uninstall:

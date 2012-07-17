vpath % ..:../core:../product:../drivers:../stm
vpath ao-make-product.5c ../util

ifndef VERSION
include ../Version
endif

CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

PDCLIB=/home/keithp/sat
C_LIB=$(PDCLIB)/lib/pdclib.a
C_INC=-I$(PDCLIB)/include

DEF_CFLAGS=-g -std=gnu99 -Os -mlittle-endian -mthumb -ffreestanding -nostdlib -I../../src/stm $(C_INC)

# to run from SRAM
LD_FLAGS_RAM=-L../stm -Wl,-Taltos-ram.ld
LD_FLAGS=-L../stm -Wl,-Tbringup.ld

CFLAGS=$(DEF_CFLAGS) -mcpu=cortex-m3 -DCONFIG_STM32L_DISCOVERY

SRC=bringup.c ao_interrupt.c
OBJ=$(SRC:.c=.o)

all: bringup-ram.elf bringup.elf

%.bin: %.elf
	$(OBJCOPY) -O binary $^ $@

bringup.elf: $(OBJ) $(C_LIB) bringup.ld
	$(CC) $(CFLAGS) $(LD_FLAGS) -o $@ $(OBJ) $(C_LIB) -lgcc

bringup-ram.elf: $(OBJ) $(C_LIB) altos-ram.ld
	$(CC) $(CFLAGS) $(LD_FLAGS_RAM) -o $@ $(OBJ) $(C_LIB) -lgcc

clean:
	rm -f *.o
	rm -rf *.elf
	rm -rf *.bin

.PHONY: all clean

install:

uninstall:

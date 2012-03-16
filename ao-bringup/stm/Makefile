CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

C_LIB=/local/src/pdclib/pdclib.a
C_INC=-I/local/src/pdclib/includes -I/local/src/pdclib/internals

DEF_CFLAGS=-g -std=gnu99 -O0 -mlittle-endian -mthumb -ffreestanding -nostdlib -I../../src/stm $(C_INC)

# to run from SRAM
LD_FLAGS_RAM=-Wl,-Taltos-ram.ld
LD_FLAGS=-Wl,-Taltos.ld

CFLAGS=$(DEF_CFLAGS) -mcpu=cortex-m3 -DCONFIG_STM32L_DISCOVERY

OBJS=bringup.o

all: bringup-ram.elf bringup.elf

%.bin: %.elf
	$(OBJCOPY) -O binary $^ $@

bringup.elf: $(OBJS) $(C_LIB) altos.ld
	$(CC) $(CFLAGS) $(LD_FLAGS) -o $@ $(OBJS) $(C_LIB) -lgcc

bringup-ram.elf: $(OBJS) $(C_LIB) altos-ram.ld
	$(CC) $(CFLAGS) $(LD_FLAGS_RAM) -o $@ $(OBJS) $(C_LIB) -lgcc

bringup.o: bringup.c
	$(CC) -c $(CFLAGS) bringup.c

clean:
	rm -rf *.elf
	rm -rf *.bin

.PHONY: all clean

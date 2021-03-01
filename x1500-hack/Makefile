# x1500-hack/Makefile
# Copyright (c) 2020 J. M. Spivey

all: total.hex attack squirt

CC = arm-none-eabi-gcc
CPU = -mcpu=cortex-m4 -mthumb
CFLAGS = -O -g -Wall -ffreestanding
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
SIZE = arm-none-eabi-size
OBJCOPY = arm-none-eabi-objcopy

%.o: %.c
	$(CC) $(CPU) $(CFLAGS) $(INCLUDE) -c $< -o $@

%.o: %.s
	$(AS) $(CPU) $< -o $@

%.elf: %.o startup.o
	$(CC) $(CPU) $(CFLAGS) -T nRF52833.ld \
		$^ -nostdlib -lgcc -o $@ -Wl,-Map,$*.map
	$(SIZE) $@

%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@

attack: attack.o
	$(OBJCOPY) -O binary attack.o attack.bin
	hexdump -v -e '/4 "%d\n"' attack.bin >attack

squirt: squirt.c
	gcc $< -o $@

# Nuke the default rules for building executables
SORRY = echo "Please say 'make $@.hex' to compile '$@'"
%: %.s; @$(SORRY)
%: %.o; @$(SORRY)

clean:
	rm -f *.hex *.elf *.map *.o attack squirt

# Don't delete intermediate files
.SECONDARY:

###

total.elf: lib.o

total.o: lib.h hardware.h
startup.o: hardware.h
lib.o: lib.h

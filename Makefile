# put your *.o targets here, make should handle the rest!

SRCS = main.c dht22.c timer.c ub_lib/stm32_ub_graphic2d.c   ub_lib/stm32_ub_lcd_ili9341.c ub_lib/stm32_ub_sdram.c       ub_lib/stm32_ub_spi5.c ub_lib/stm32_ub_font.c
#SRCS = main.c system.c

# all the files will be generated with this name (main.elf, main.bin, main.hex, etc)

PROJ_NAME=main

# Put your stlink folder here so make burn will work.

#STLINK=/mnt/share/Programming/embedded/stm32/stlink

# that's it, no need to change anything below this line!

###################################################

CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
#CCDIR = /Users/rene/Downloads/gcc-arm-none-eabi-4_7-2013q3/bin

CFLAGS  = -g -Wall -Tstm32_flash.ld -std=gnu99# -O3 #-finline-functions  #-O2 -funswitch-loops -fpredictive-commoning -fgcse-after-reload -ftree-slp-vectorize -fvect-cost-model -fipa-cp-clone 
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork -nostartfiles
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16 -nostartfiles -fsingle-precision-constant

###################################################

vpath %.c src
vpath %.a lib

ROOT=$(shell pwd)

#CFLAGS += -Iinc -Ilib -Ilib/inc
#CFLAGS += -Ilib/inc/core -Ilib/inc/peripherals
#CFLAGS += -Iub_lib
CFLAGS += -Icmsis_boot
CFLAGS += -Icmsis
CFLAGS += -Lcmsis_lib/include
CFLAGS += -Icmsis_lib/include
CFLAGS += -Iub_lib
CFLAGS += -Lcmsis_lib
CFLAGS += -DSTM32F429_439xx

SRCS += cmsis_boot/startup/startup_stm32f4xx.c
SRCS += cmsis_boot/system_stm32f4xx.c
SRCS += stm32f4xx_it.c
#SRCS += dht22.c
#SRCS += stm32f4xx_dma2d.c stm32f4xx_fmc.c   stm32f4xx_gpio.c  stm32f4xx_ltdc.c  stm32f4xx_rcc.c   stm32f4xx_spi.c

OBJS = $(SRCS:.c=.o)

###################################################

.PHONY: lib proj

all: proj

again: clean all

# Flash the STM32F4
burn: main.elf
	st-flash --reset write $(PROJ_NAME).bin 0x8000000

# Create tags; assumes ctags exists
ctags:
	ctags -R --exclude=*cm0.h --exclude=*cm3.h .

#lib:
#	$(MAKE) -C lib

proj: 	$(PROJ_NAME).elf
	
term:	$(PROJ_NAME).elf
	st-term `arm-none-eabi-nm main.elf | grep stlinky_term | awk '{print $$1}'`

$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ -Llib -lstm32f4
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin

clean:
	rm -f *.o *.i
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).bin

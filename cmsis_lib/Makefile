CC=arm-none-eabi-gcc
AR=arm-none-eabi-ar
#CCDIR = /Users/rene/Downloads/gcc-arm-none-eabi-4_7-2013q3/bin

###########################################

vpath %.c source

CFLAGS  = -g -O2 -Wall
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -ffreestanding -nostdlib
CFLAGS += -Iinclude -I../cmsis_boot -I../cmsis -DSTM32F429_439xx -DUSE_STDPERIPH_DRIVER

#SRCS  = stm32f4_discovery.c
SRCS = stm32f4xx_dma2d.c stm32f4xx_fmc.c   stm32f4xx_gpio.c  stm32f4xx_ltdc.c  stm32f4xx_rcc.c   stm32f4xx_spi.c

OBJS = $(SRCS:.c=.o)

.PHONY: libstm32f4.a

all: libstm32f4.a

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

libstm32f4.a: $(OBJS)
	$(AR) -r $@ $(OBJS)

clean:
	rm -f $(OBJS) libstm32f4.a

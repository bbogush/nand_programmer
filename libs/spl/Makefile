LIB_NAME=libstm32f10x.a

SRC_DIR=STM32F10x_StdPeriph_Driver/src/
OBJ_DIR=STM32F10x_StdPeriph_Driver/obj/

TOOLCHAIN=../../../compiler/gcc-arm-none-eabi-4_9-2015q1/bin/arm-none-eabi-

CC=$(TOOLCHAIN)gcc
AR=$(TOOLCHAIN)ar

CHIP=STM32F10X_MD

CONFIG_FILE=stm32f10x_conf.h

INCLUDES=-include$(CONFIG_FILE)
INCLUDES+=-ISTM32F10x_StdPeriph_Driver/inc/
INCLUDES+=-ICMSIS/CM3/DeviceSupport/ST/STM32F10x/
INCLUDES+=-ICMSIS/CM3/CoreSupport/

CFLAGS=-g -Wall -Werror
CFLAGS+=-D$(CHIP) $(INCLUDES) -MMD -MP
CFLAGS+=-mcpu=cortex-m3 -mthumb
CFLAGS+=-ffreestanding -nostdlib

vpath %.c $(SRC_DIR)

SRCS=misc.c stm32f10x_cec.c stm32f10x_dma.c stm32f10x_gpio.c stm32f10x_rcc.c \
  stm32f10x_tim.c stm32f10x_adc.c stm32f10x_crc.c stm32f10x_exti.c \
  stm32f10x_i2c.c stm32f10x_rtc.c stm32f10x_usart.c stm32f10x_bkp.c \
  stm32f10x_dac.c stm32f10x_flash.c stm32f10x_iwdg.c stm32f10x_sdio.c \
  stm32f10x_wwdg.c stm32f10x_can.c stm32f10x_dbgmcu.c stm32f10x_fsmc.c \
  stm32f10x_pwr.c stm32f10x_spi.c
OBJS=$(addprefix $(OBJ_DIR),$(SRCS:.c=.o))
DEPS=$(OBJS:%.o=%.d)

all: dirs $(LIB_NAME)

dirs:
	mkdir -p $(OBJ_DIR)

$(LIB_NAME): $(OBJS)
	$(AR) -r $@ $^

$(OBJ_DIR)%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(LIB_NAME) $(OBJ_DIR)


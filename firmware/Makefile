# Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3.

PROG_NAME=prog

SRC_DIR=./
USB_DIR=usb_cdc/
OBJ_DIR=obj/
EXTRA_DIR=../extra/
PROG=$(OBJ_DIR)$(PROG_NAME)

SPL_PATH=libs/spl/
SPL_DEVICE_SUPPORT_PATH=$(SPL_PATH)CMSIS/CM3/DeviceSupport/ST/STM32F10x/
SPL_CORE_SUPPORT=$(SPL_PATH)CMSIS/CM3/CoreSupport/
SPL_PERIPH_PATH=$(SPL_PATH)STM32F10x_StdPeriph_Driver/
SPL_USB_PATH=$(SPL_PATH)STM32_USB-FS-Device_Driver/
SPL_LIB=stm32f10x
SPL_CONFIG_FILE=$(SPL_PATH)stm32f10x_conf.h
SPL_FLAGS=-DSTM32F10X_HD

TOOLCHAIN=../../compiler/gcc-arm-none-eabi-4_9-2015q1/bin/arm-none-eabi-

CC=$(TOOLCHAIN)gcc
OBJCOPY=$(TOOLCHAIN)objcopy
OBJDUMP=$(TOOLCHAIN)objdump
SIZE=$(TOOLCHAIN)size

INCLUDES=-include$(SPL_CONFIG_FILE)
INCLUDES+=-I$(SPL_CORE_SUPPORT)
INCLUDES+=-I$(SPL_DEVICE_SUPPORT_PATH)
INCLUDES+=-I$(SPL_PATH)
INCLUDES+=-I$(SPL_PERIPH_PATH)inc
INCLUDES+=-I$(SPL_USB_PATH)inc
INCLUDES+=-I$(SRC_DIR)
INCLUDES+=-I$(USB_DIR)

CFLAGS=-g -Wall -Werror -O3
CFLAGS+=$(INCLUDES) -MMD -MP
CFLAGS+=-ffunction-sections -fdata-sections
CFLAGS+=-mcpu=cortex-m3 -mthumb
CFLAGS+=$(SPL_FLAGS)

LDFLAGS=-mcpu=cortex-m3 -mthumb -Wl,--gc-sections -Wl,-Map=$(PROG).map

vpath %.c $(SRC_DIR) $(SPL_DEVICE_SUPPORT_PATH) $(SRC_BSP_DIR) $(USB_DIR)
vpath %.s $(SRC_DIR)

STARTUP=startup_stm32f10x_hd.s
USB_SRCS=hw_config.c stm32_it.c usb_prop.c usb_desc.c usb_istr.c usb_pwr.c \
  usb_endp.c usb.c


SRCS=main.c system_stm32f10x.c syscalls.c fsmc_nand.c chip_db.c led.c uart.c \
  jtag.c cdc.c nand_programmer.c nand_bad_block.c $(USB_SRCS)

OBJS=$(addprefix $(OBJ_DIR),$(SRCS:.c=.o)) \
  $(addprefix $(OBJ_DIR),$(STARTUP:.s=.o))
DEPS=$(OBJS:%.o=%.d)

LINKER_SCRIPT=$(SRC_DIR)stm32_flash.ld

all: lib dirs $(PROG).elf

lib:
	$(MAKE) -C $(SPL_PATH)

dirs:
	mkdir -p $(OBJ_DIR)

$(PROG).elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ -L$(SPL_PATH) -l$(SPL_LIB) -T$(LINKER_SCRIPT)
	$(OBJCOPY) -O ihex $(PROG).elf $(PROG).hex
	$(OBJCOPY) -O binary $(PROG).elf $(PROG).bin
	$(OBJDUMP) -St $(PROG).elf > $(PROG).lst
	$(SIZE) $(PROG).elf

$(OBJ_DIR)%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ 

$(OBJ_DIR)%.o: %.s
	$(CC) -c $(CFLAGS) $< -o $@

-include $(DEPS)

clean:
	rm -rf $(OBJ_DIR)

distclean: clean
	$(MAKE) -C $(SPL_PATH) clean

program:
	$(EXTRA_DIR)program.sh $(EXTRA_DIR)openocd.cfg $(PROG).hex

erase:
	$(EXTRA_DIR)erase.sh $(EXTRA_DIR)openocd.cfg


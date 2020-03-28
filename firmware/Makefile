# Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3.

BOOT_NAME=bootloader_fw.hex
PROG_NAME=app_fw.hex
GEN_NAME=nando_fw

BOOT_DIR=bootloader/
PROG_DIR=programmer/

BOOT_OBJ_DIR=$(BOOT_DIR)obj/
PROG_OBJ_DIR=$(PROG_DIR)obj/

BOOT_PATH=$(BOOT_OBJ_DIR)$(BOOT_NAME)
PROG_PATH=$(PROG_OBJ_DIR)$(PROG_NAME)

OBJ_DIR=obj/

TOOLCHAIN=../../../compiler/gcc-arm-none-eabi-4_9-2015q1/bin/arm-none-eabi-
OBJCOPY=$(TOOLCHAIN)objcopy

all:
	$(MAKE) -C $(BOOT_DIR)
	$(MAKE) -C $(PROG_DIR)
	mkdir -p $(OBJ_DIR)
	cp $(BOOT_PATH) $(OBJ_DIR)
	cp $(PROG_PATH) $(OBJ_DIR)
	cd $(OBJ_DIR) && sed -i '$$d' $(BOOT_NAME) && \
	cat $(BOOT_NAME) $(PROG_NAME) > $(GEN_NAME).hex && \
	$(OBJCOPY) --input-target=ihex --output-target=binary $(GEN_NAME).hex \
	  $(GEN_NAME).bin

clean:
	$(MAKE) -C $(BOOT_DIR) clean
	$(MAKE) -C $(PROG_DIR) clean
	rm -rf obj/

distclean:
	$(MAKE) -C $(BOOT_DIR) distclean
	$(MAKE) -C $(PROG_DIR) distclean
	rm -rf obj/

erase:
	st-flash erase

program:
	st-flash write $(OBJ_DIR)$(GEN_NAME).bin 0x08000000

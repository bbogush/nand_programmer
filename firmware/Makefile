# Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3.

BOOT_DIR=bootloader
PROG_DIR=programmer

all:
	$(MAKE) -C $(BOOT_DIR)
	$(MAKE) -C $(PROG_DIR)

clean:
	$(MAKE) -C $(BOOT_DIR) clean
	$(MAKE) -C $(PROG_DIR) clean

distclean:
	$(MAKE) -C $(BOOT_DIR) distclean
	$(MAKE) -C $(PROG_DIR) distclean

program:
	$(MAKE) -C $(BOOT_DIR) program
	$(MAKE) -C $(PROG_DIR) program

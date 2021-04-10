# Copyright (C) 2020 NANDO authors
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3.

all:
ifeq ($(OS), Windows_NT)
	$(MAKE) -f Makefile.windows all
else
	$(MAKE) -f Makefile.linux all
endif

clean:
ifeq ($(OS), Windows_NT)
	$(MAKE) -f Makefile.windows clean
else
	$(MAKE) -f Makefile.linux clean
endif

distclean:
ifeq ($(OS), Windows_NT)
	$(MAKE) -f Makefile.windows distclean
else
	$(MAKE) -f Makefile.linux distclean
endif

/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef _NAND_BAD_BLOCK_H_
#define _NAND_BAD_BLOCK_H_

#include <stdint.h>
#include <stdbool.h>

void nand_bad_block_table_init();
int nand_bad_block_table_add(uint32_t addr);
bool nand_bad_block_table_lookup(uint32_t addr);

#endif

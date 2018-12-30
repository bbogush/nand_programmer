/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "nand_bad_block.h"
#include <string.h>

#define NAND_BAD_BLOCK_TABLE_SIZE 20

static uint32_t nand_bad_block_table_count;
static uint32_t nand_bad_block_table[NAND_BAD_BLOCK_TABLE_SIZE];

void nand_bad_block_table_init()
{
    memset(nand_bad_block_table, 0, NAND_BAD_BLOCK_TABLE_SIZE);
    nand_bad_block_table_count = 0;
}

int nand_bad_block_table_add(uint32_t addr)
{
    if (nand_bad_block_table_count == NAND_BAD_BLOCK_TABLE_SIZE)
        return -1;

    nand_bad_block_table[nand_bad_block_table_count++] = addr;
    return 0;
}

bool nand_bad_block_table_lookup(uint32_t addr)
{
    uint32_t i;

    for (i = 0; i < nand_bad_block_table_count; i++)
    {
        if (nand_bad_block_table[i] == addr)
            return true;
    }

    return false;
}

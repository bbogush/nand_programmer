/*  Copyright (C) 2020 NANDO authors
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
    memset(nand_bad_block_table, 0, sizeof(nand_bad_block_table));
    nand_bad_block_table_count = 0;
}

int nand_bad_block_table_add(uint32_t page)
{
    if (nand_bad_block_table_count == NAND_BAD_BLOCK_TABLE_SIZE)
        return -1;

    nand_bad_block_table[nand_bad_block_table_count++] = page;
    return 0;
}

bool nand_bad_block_table_lookup(uint32_t page)
{
    uint32_t i;

    for (i = 0; i < nand_bad_block_table_count; i++)
    {
        if (nand_bad_block_table[i] == page)
            return true;
    }

    return false;
}

void *nand_bad_block_table_iter_alloc(uint32_t *page)
{
    if (!nand_bad_block_table_count)
        return NULL;

    *page = nand_bad_block_table[0];

    return &nand_bad_block_table[0];
}

void *nand_bad_block_table_iter_next(void *iter, uint32_t *page)
{
    uint32_t *bbt_iter = iter;

    if (!bbt_iter)
       return NULL;

    bbt_iter++;
       
    if (bbt_iter - &nand_bad_block_table[0] >= nand_bad_block_table_count)
        return NULL;

    *page = *bbt_iter;

    return bbt_iter;
}

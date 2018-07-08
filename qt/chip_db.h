/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef CHIP_DB_H
#define CHIP_DB_H

#include <cstdint>

#define MAX_CHIP_NAME_LEN 16

enum
{
    CHIP_ID_NONE = 0,
    CHIP_ID_K9F2G08U0C = 1,
    CHIP_ID_LAST = 2,
};

typedef struct
{
    uint32_t id;
    char name[MAX_CHIP_NAME_LEN];
    uint32_t page_size;
    uint32_t block_size;
    uint32_t size;
} ChipInfo;

uint32_t chipDbGet(ChipInfo *&db);
ChipInfo *chipInfoGetByName(char *name);
ChipInfo *chipInfoGetById(uint32_t id);
uint32_t chipPageSizeGet(uint32_t id);

#endif // CHIP_DB_H


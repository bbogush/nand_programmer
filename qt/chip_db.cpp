/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "chip_db.h"
#include <cstring>

static ChipInfo chipDB[] =
{
    /* id, name, pageSize, blockSize, size, tCS, tCLS, tALS, tCLR, tAR, tWP, tRP, tDS, tCH, tCLH, tALH, tWC, tRC, tREA */
    { CHIP_ID_NONE, "No Chip", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { CHIP_ID_K9F2G08U0C, "K9F2G08U0C", 0x800, 0x20000, 0x10000000, 20, 12, 12, 10, 10, 12, 12, 12, 5, 5, 5, 25, 25, 20 },
};

uint32_t chipDbGet(ChipInfo *&db)
{
    db = chipDB;

    return CHIP_ID_LAST;
}

ChipInfo *chipInfoGetByName(char *name)
{
    for (int id = 0; id < CHIP_ID_LAST; id++)
    {
        if (!strcmp(name, chipDB[id].name))
            return &chipDB[id];
    }

    return 0;
}

ChipInfo *chipInfoGetById(uint32_t id)
{
    if (id == CHIP_ID_NONE || id >= CHIP_ID_LAST)
        return NULL;

    return &chipDB[id];
}

uint32_t chipPageSizeGet(uint32_t id)
{
    ChipInfo *info = chipInfoGetById(id);

    return info ? info->pageSize : 0;
}

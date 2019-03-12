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
    uint32_t pageSize;
    uint32_t blockSize;
    uint32_t size;
    uint32_t tCS;
    uint32_t tCLS;
    uint32_t tALS;
    uint32_t tCLR;
    uint32_t tAR;
    uint32_t tWP;
    uint32_t tRP;
    uint32_t tDS;
    uint32_t tCH;
    uint32_t tCLH;
    uint32_t tALH;
    uint32_t tWC;
    uint32_t tRC;
    uint32_t tREA;
} ChipInfo;

uint32_t chipDbGet(ChipInfo *&db);
ChipInfo *chipInfoGetByName(char *name);
ChipInfo *chipInfoGetById(uint32_t id);
uint32_t chipPageSizeGet(uint32_t id);

#endif // CHIP_DB_H


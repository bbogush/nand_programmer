/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "chip_db.h"

QString ChipDb::getChipName(int chipIndex)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    return ci ? ci->name : QString();
}

int ChipDb::setChipName(int chipIndex, const QString &name)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    if (!ci)
        return -1;

    ci->name = name;

    return 0;
}

uint32_t ChipDb::getPageSize(int chipIndex)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    return ci ? ci->pageSize : 0;
}

int ChipDb::setPageSize(int chipIndex, uint32_t pageSize)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    if (!ci)
        return -1;

    ci->pageSize = pageSize;

    return 0;
}

uint32_t ChipDb::getBlockSize(int chipIndex)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    return ci ? ci->blockSize : 0;
}

int ChipDb::setBlockSize(int chipIndex, uint32_t blockSize)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    if (!ci)
        return -1;

    ci->blockSize = blockSize;

    return 0;
}

uint32_t ChipDb::getTotalSize(int chipIndex)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    return ci ? ci->totalSize : 0;
}

int ChipDb::setTotalSize(int chipIndex, uint32_t totalSize)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    if (!ci)
        return -1;

    ci->totalSize = totalSize;

    return 0;
}

uint32_t ChipDb::getSpareSize(int chipIndex)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    return ci ? ci->spareSize : 0;
}

int ChipDb::setSpareSize(int chipIndex, uint32_t spareSize)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    if (!ci)
        return -1;

    ci->spareSize = spareSize;

    return 0;
}

uint8_t ChipDb::getBBMarkOffset(int chipIndex)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    return ci ? ci->bbMarkOffset : 0;
}

int ChipDb::setBBMarkOffset(int chipIndex, uint8_t bbMarkOffset)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    if (!ci)
        return -1;

    ci->bbMarkOffset = bbMarkOffset;

    return 0;
}

/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "chip_db.h"

QString ChipDb::getChipName(int chipIndex)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    return ci ? ci->getName() : QString();
}

int ChipDb::setChipName(int chipIndex, const QString &name)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    if (!ci)
        return -1;

    ci->setName(name);

    return 0;
}

uint32_t ChipDb::getPageSize(int chipIndex)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    return ci ? ci->getPageSize() : 0;
}

int ChipDb::setPageSize(int chipIndex, uint32_t pageSize)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    if (!ci)
        return -1;

    ci->setPageSize(pageSize);

    return 0;
}

uint32_t ChipDb::getBlockSize(int chipIndex)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    return ci ? ci->getBlockSize() : 0;
}

int ChipDb::setBlockSize(int chipIndex, uint32_t blockSize)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    if (!ci)
        return -1;

    ci->setBlockSize(blockSize);

    return 0;
}

uint32_t ChipDb::getTotalSize(int chipIndex)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    return ci ? ci->getTotalSize() : 0;
}

int ChipDb::setTotalSize(int chipIndex, uint32_t totalSize)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    if (!ci)
        return -1;

    ci->setTotalSize(totalSize);

    return 0;
}

uint32_t ChipDb::getSpareSize(int chipIndex)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    return ci ? ci->getSpareSize() : 0;
}

int ChipDb::setSpareSize(int chipIndex, uint32_t spareSize)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    if (!ci)
        return -1;

    ci->setSpareSize(spareSize);

    return 0;
}

uint8_t ChipDb::getBBMarkOffset(int chipIndex)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    return ci ? ci->getBBMarkOffset() : 0;
}

int ChipDb::setBBMarkOffset(int chipIndex, uint8_t bbMarkOffset)
{
    ChipInfo *ci = getChipInfo(chipIndex);

    if (!ci)
        return -1;

    ci->setBBMarkOffset(bbMarkOffset);

    return 0;
}

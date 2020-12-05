/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "chip_info.h"

ChipInfo::ChipInfo()
{
    name = "Unknown";
    pageSize = 0;
    blockSize = 0;
    totalSize = 0;
    spareSize = 0;
    bbMarkOffset = 0;
    hal = CHIP_HAL_LAST;
}

ChipInfo::~ChipInfo()
{
}

const QString &ChipInfo::getName()
{
    return name;
}

void ChipInfo::setName(const QString &name)
{
    this->name = name;
}

uint32_t ChipInfo::getPageSize()
{
    return pageSize;
}

void ChipInfo::setPageSize(uint32_t pageSize)
{
    this->pageSize = pageSize;
}

uint32_t ChipInfo::getBlockSize()
{
    return blockSize;
}

void ChipInfo::setBlockSize(uint32_t blockSize)
{
    this->blockSize = blockSize;
}

uint32_t ChipInfo::getTotalSize()
{
    return totalSize;
}

void ChipInfo::setTotalSize(uint32_t totalSize)
{
    this->totalSize = totalSize;
}

uint32_t ChipInfo::getSpareSize()
{
    return spareSize;
}

void ChipInfo::setSpareSize(uint32_t spareSize)
{
    this->spareSize = spareSize;
}

uint8_t ChipInfo::getBBMarkOffset()
{
    return bbMarkOffset;
}

void ChipInfo::setBBMarkOffset(uint8_t offset)
{
    this->bbMarkOffset = offset;
}

uint8_t ChipInfo::getHal()
{
    return hal;
}

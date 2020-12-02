/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef CHIP_DB_H
#define CHIP_DB_H

#include "chip_info.h"
#include <QStringList>

enum CHIP_HAL
{
    CHIP_HAL_PARALLEL = 0,
    CHIP_HAL_SPI = 1
};

class ChipDb
{
public:
    virtual QStringList getNames() = 0;
    virtual ChipInfo *chipInfoGetById(int id) = 0;
    virtual QString getNameByChipId(uint32_t id1, uint32_t id2,
        uint32_t id3, uint32_t id4, uint32_t id5) = 0;
    virtual uint32_t pageSizeGetByName(const QString &name) = 0;
    virtual uint32_t extendedPageSizeGetByName(const QString &name) = 0;
    virtual uint32_t totalSizeGetByName(const QString &name) = 0;
    virtual uint32_t extendedTotalSizeGetByName(const QString &name) = 0;
    virtual uint8_t getHal() = 0;
    virtual ChipInfo *getChipInfo(int chipIndex) = 0;
    QString getChipName(int chipIndex);
    int setChipName(int chipIndex, const QString &name);
    uint32_t getPageSize(int chipIndex);
    int setPageSize(int chipIndex, uint32_t pageSize);
    uint32_t getBlockSize(int chipIndex);
    int setBlockSize(int chipIndex, uint32_t blockSize);
    uint32_t getTotalSize(int chipIndex);
    int setTotalSize(int chipIndex, uint32_t totalSize);
    uint32_t getSpareSize(int chipIndex);
    int setSpareSize(int chipIndex, uint32_t spareSize);
    uint32_t getBBMarkOffset(int chipIndex);
    int setBBMarkOffset(int chipIndex, uint32_t bbMarkOffset);
};

#endif // CHIP_DB_H

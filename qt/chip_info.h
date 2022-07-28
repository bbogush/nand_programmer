/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef CHIP_INFO_H
#define CHIP_INFO_H

#include <QString>

class ChipInfo
{
protected:
    enum ChipHal
    {
        CHIP_HAL_PARALLEL = 0,
        CHIP_HAL_SPI = 1,
        CHIP_HAL_LAST
    };

    QString name;
    uint32_t pageSize;
    uint32_t blockSize;
    quint64 totalSize;
    uint32_t spareSize;
    uint8_t bbMarkOffset;
    uint32_t hal;

public:
    ChipInfo();
    virtual ~ChipInfo();
    const QString &getName();
    void setName(const QString &name);
    uint32_t getPageSize();
    void setPageSize(uint32_t pageSize);
    uint32_t getBlockSize();
    void setBlockSize(uint32_t blockSize);
    quint64 getTotalSize();
    void setTotalSize(quint64 totalSize);
    uint32_t getSpareSize();
    void setSpareSize(uint32_t spareSize);
    uint8_t getBBMarkOffset();
    void setBBMarkOffset(uint8_t offset);
    uint8_t getHal();
    virtual const QByteArray &getHalConf() = 0;
    virtual quint64 getParam(uint32_t num) = 0;
    virtual int setParam(uint32_t num, quint64 value) = 0;
};

#endif // CHIP_INFO_H

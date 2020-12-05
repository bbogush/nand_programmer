/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef SPI_CHIP_INFO_H
#define SPI_CHIP_INFO_H

#include "chip_info.h"

class SpiChipInfo : public ChipInfo
{
public:
    enum
    {
        CHIP_PARAM_ID1,
        CHIP_PARAM_ID2,
        CHIP_PARAM_ID3,
        CHIP_PARAM_ID4,
        CHIP_PARAM_ID5,
        CHIP_PARAM_NUM,
    };

private:
    QByteArray halConf;
    uint32_t params[CHIP_PARAM_NUM];

public:
    SpiChipInfo();
    virtual ~SpiChipInfo();
    const QByteArray &getHalConf() override;
    uint32_t getParam(uint32_t num);
    int setParam(uint32_t num, uint32_t value);
};

#endif // SPI_CHIP_INFO_H

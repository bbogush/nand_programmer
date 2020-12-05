/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "spi_chip_info.h"

typedef struct __attribute__((__packed__))
{
    uint32_t dummy;
} Conf;

SpiChipInfo::SpiChipInfo()
{
    hal = CHIP_HAL_SPI;
    spareSize = 0;
    bbMarkOffset = 0;
}

SpiChipInfo::~SpiChipInfo()
{
}

const QByteArray &SpiChipInfo::getHalConf()
{
    Conf conf;

    halConf.clear();
    halConf.append(reinterpret_cast<const char *>(&conf), sizeof(conf));

    return halConf;
}

uint32_t SpiChipInfo::getParam(uint32_t num)
{
    if (num >= CHIP_PARAM_NUM)
        return 0;

    return params[num];
}

int SpiChipInfo::setParam(uint32_t num, uint32_t value)
{
    if (num >= CHIP_PARAM_NUM)
        return -1;

    params[num] = value;

    return 0;
}

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

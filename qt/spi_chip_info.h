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
    SpiChipInfo();
    virtual ~SpiChipInfo();
    const QByteArray &getHalConf() override;
};

#endif // SPI_CHIP_INFO_H

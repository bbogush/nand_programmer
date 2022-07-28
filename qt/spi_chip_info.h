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
        CHIP_PARAM_PAGE_OFF,
        CHIP_PARAM_READ_CMD,
        CHIP_PARAM_READ_ID_CMD,
        CHIP_PARAM_WRITE_CMD,
        CHIP_PARAM_WRITE_EN_CMD,
        CHIP_PARAM_ERASE_CMD,
        CHIP_PARAM_STATUS_CMD,
        CHIP_PARAM_BUSY_BIT,
        CHIP_PARAM_BUSY_STATE,
        CHIP_PARAM_FREQ,
        CHIP_PARAM_ID1,
        CHIP_PARAM_ID2,
        CHIP_PARAM_ID3,
        CHIP_PARAM_ID4,
        CHIP_PARAM_ID5,
        CHIP_PARAM_NUM,
    };

private:
    QByteArray halConf;
    quint64 params[CHIP_PARAM_NUM] = {};

public:
    SpiChipInfo();
    virtual ~SpiChipInfo();
    const QByteArray &getHalConf() override;
    quint64 getParam(uint32_t num) override;
    int setParam(uint32_t num, quint64 value) override;
};

#endif // SPI_CHIP_INFO_H

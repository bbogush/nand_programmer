/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "spi_chip_info.h"

typedef struct __attribute__((__packed__))
{
    uint8_t page_offset;
    uint8_t read_cmd;
    uint8_t read_id_cmd;
    uint8_t write_cmd;
    uint8_t erase_cmd;
    uint8_t status_cmd;
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

    conf.page_offset = static_cast<uint8_t>(params[CHIP_PARAM_PAGE_OFF]);
    conf.read_cmd = static_cast<uint8_t>(params[CHIP_PARAM_READ_CMD]);
    conf.read_id_cmd = static_cast<uint8_t>(params[CHIP_PARAM_READ_ID_CMD]);
    conf.write_cmd = static_cast<uint8_t>(params[CHIP_PARAM_WRITE_CMD]);
    conf.erase_cmd = static_cast<uint8_t>(params[CHIP_PARAM_ERASE_CMD]);
    conf.status_cmd = static_cast<uint8_t>(params[CHIP_PARAM_STATUS_CMD]);

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

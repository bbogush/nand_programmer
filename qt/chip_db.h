/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef CHIP_DB_H
#define CHIP_DB_H

#include <QStringList>

enum
{
    CHIP_PARAM_NAME,
    CHIP_PARAM_PAGE_SIZE,
    CHIP_PARAM_BLOCK_SIZE,
    CHIP_PARAM_TOTAL_SIZE,
    CHIP_PARAM_SPARE_SIZE,
    CHIP_PARAM_T_CS,
    CHIP_PARAM_T_CLS,
    CHIP_PARAM_T_ALS,
    CHIP_PARAM_T_CLR,
    CHIP_PARAM_T_AR,
    CHIP_PARAM_T_WP,
    CHIP_PARAM_T_RP,
    CHIP_PARAM_T_DS,
    CHIP_PARAM_T_CH,
    CHIP_PARAM_T_CLH,
    CHIP_PARAM_T_ALH,
    CHIP_PARAM_T_WC,
    CHIP_PARAM_T_RC,
    CHIP_PARAM_T_REA,
    CHIP_PARAM_ROW_CYCLES,
    CHIP_PARAM_COL_CYCLES,
    CHIP_PARAM_READ1_CMD,
    CHIP_PARAM_READ2_CMD,
    CHIP_PARAM_READ_SPARE_CMD,
    CHIP_PARAM_READ_ID_CMD,
    CHIP_PARAM_RESET_CMD,
    CHIP_PARAM_WRITE1_CMD,
    CHIP_PARAM_WRITE2_CMD,
    CHIP_PARAM_ERASE1_CMD,
    CHIP_PARAM_ERASE2_CMD,
    CHIP_PARAM_STATUS_CMD,
    CHIP_PARAM_BB_MARK_OFF,
    CHIP_PARAM_ID1,
    CHIP_PARAM_ID2,
    CHIP_PARAM_ID3,
    CHIP_PARAM_ID4,
    CHIP_PARAM_ID5,
    CHIP_PARAM_NUM,
};

typedef struct
{
    uint32_t id;
    QString name;
    uint32_t params[CHIP_PARAM_NUM];
} ChipInfo;

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
    virtual uint8_t getHal() = 0;
};

#endif // CHIP_DB_H

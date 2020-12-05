/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef PARALLEL_CHIP_INFO_H
#define PARALLEL_CHIP_INFO_H

#include "chip_info.h"

class ParallelChipInfo : public ChipInfo
{
public:
    enum
    {
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

private:
    typedef struct
    {
        uint8_t setupTime;
        uint8_t waitSetupTime;
        uint8_t holdSetupTime;
        uint8_t hiZSetupTime;
        uint8_t clrSetupTime;
        uint8_t arSetupTime;
    } StmParams;

    QByteArray halConf;
    uint32_t params[CHIP_PARAM_NUM];

    void chipInfoToStmParams(StmParams *stmParams);

public:
    ParallelChipInfo();
    virtual ~ParallelChipInfo();
    const QByteArray &getHalConf() override;
    uint32_t getParam(uint32_t num);
    int setParam(uint32_t num, uint32_t value);
};

#endif // PARALLEL_CHIP_INFO_H

/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "parallel_chip_info.h"
#include <stdint.h>
#include <algorithm>
#include <array>
#include <math.h>

typedef struct __attribute__((__packed__))
{
    uint8_t setupTime;
    uint8_t waitSetupTime;
    uint8_t holdSetupTime;
    uint8_t hiZSetupTime;
    uint8_t clrSetupTime;
    uint8_t arSetupTime;
    uint8_t rowCycles;
    uint8_t colCycles;
    uint8_t read1Cmd;
    uint8_t read2Cmd;
    uint8_t readSpareCmd;
    uint8_t readIdCmd;
    uint8_t resetCmd;
    uint8_t write1Cmd;
    uint8_t write2Cmd;
    uint8_t erase1Cmd;
    uint8_t erase2Cmd;
    uint8_t statusCmd;
    uint8_t setFeaturesCmd;
    uint8_t enableEccAddr;
    uint8_t enableEccValue;
    uint8_t disableEccValue;
} ParallelChipConf;

ParallelChipInfo::ParallelChipInfo()
{
    hal = CHIP_HAL_PARALLEL;
}

ParallelChipInfo::~ParallelChipInfo()
{
}

/* Convert NAND parameters to STM32 FSMC parameters.
 * See Application Note AN2784.
 */
void ParallelChipInfo::chipInfoToStmParams(StmParams *stmParams)
{
    double setupTime, waitSetupTime, hiZSetupTime, holdSetupTime, arSetupTime,
        clrSetupTime;
    const double tHCLK = 13.88; /* 1 / 72MHz */
    const double tsuD_NOE = 25;
    std::array<quint64, 5> setupArr = { params[CHIP_PARAM_T_CS],
        params[CHIP_PARAM_T_CLS], params[CHIP_PARAM_T_ALS],
        params[CHIP_PARAM_T_CLR], params[CHIP_PARAM_T_AR] };
    std::array<quint64, 3> hiZArr = { params[CHIP_PARAM_T_CS],
        params[CHIP_PARAM_T_ALS], params[CHIP_PARAM_T_CLS] };
    std::array<quint64, 3> holdArr = { params[CHIP_PARAM_T_CH],
        params[CHIP_PARAM_T_CLH], params[CHIP_PARAM_T_ALH] };

    /* (SET + 1) * tHCLK >= max(tCS, tCLS, tALS, tCLR, tAR) - tWP */
    setupTime = *std::max_element(setupArr.begin(), setupArr.end()) -
        params[CHIP_PARAM_T_WP];
    setupTime = setupTime / tHCLK - 1;
    /* K9F2G08U0C requires at least 1 tick */
    setupTime = setupTime <= 0 ? 1 : ceil(setupTime);
    stmParams->setupTime = static_cast<uint8_t>(setupTime);

    /* (WAIT + 1) * tHCLK >= max(tWP, tRP) */
    waitSetupTime = std::max(params[CHIP_PARAM_T_WP], params[CHIP_PARAM_T_RP]);
    waitSetupTime = waitSetupTime / tHCLK - 1;
    waitSetupTime = waitSetupTime <= 0 ? 0 : ceil(waitSetupTime);
    stmParams->waitSetupTime = static_cast<uint8_t>(waitSetupTime);
    /* (WAIT + 1) * tHCLK >= tREA + tsuD_NOE */
    waitSetupTime = params[CHIP_PARAM_T_REA] + tsuD_NOE;
    waitSetupTime = waitSetupTime / tHCLK - 1;
    waitSetupTime = waitSetupTime <= 0 ? 0 : ceil(waitSetupTime);
    if (waitSetupTime > stmParams->waitSetupTime)
        stmParams->waitSetupTime = static_cast<uint8_t>(waitSetupTime);

    /* (HIZ + 1) * tHCLK >= max(tCH, tALS, tCLS) + (tWP - tDS) */
    hiZSetupTime = *std::max_element(hiZArr.begin(), hiZArr.end());
    hiZSetupTime += params[CHIP_PARAM_T_WP] - params[CHIP_PARAM_T_DS];
    hiZSetupTime = hiZSetupTime / tHCLK - 1;
    hiZSetupTime = hiZSetupTime <= 0 ? 0 : ceil(hiZSetupTime);
    stmParams->hiZSetupTime = static_cast<uint8_t>(hiZSetupTime);

    /* (HOLD + 1) * tHCLK >= max(tCH, tCLH, tALH) */
    holdSetupTime =*std::max_element(holdArr.begin(), holdArr.end());
    holdSetupTime = holdSetupTime / tHCLK - 1;
    /* K9F2G08U0C requires at least 2 tick */
    holdSetupTime = holdSetupTime <= 0 ? 2 : ceil(holdSetupTime);
    stmParams->holdSetupTime = static_cast<uint8_t>(holdSetupTime);

    /* ((WAIT + 1) + (HOLD + 1) + (SET + 1)) * tHCLK >= max(tWC, tRC) */
    while (((stmParams->setupTime + 1) + (stmParams->waitSetupTime + 1) +
        (stmParams->holdSetupTime + 1)) * tHCLK <
        std::max(params[CHIP_PARAM_T_WC], params[CHIP_PARAM_T_RC]))
    {
        stmParams->setupTime++;
    }

    /* RM0008 Reference manual
       In any case, it turns out 0 */
    /* t_ar = (TAR + SET + 4) × THCLK
       t_ar / THCLK - 4 - SET  = TAR */
    arSetupTime = params[CHIP_PARAM_T_AR] / tHCLK - 4 - stmParams->setupTime;
    arSetupTime = arSetupTime <= 0 ? 0 : ceil(arSetupTime);
    stmParams->arSetupTime = static_cast<uint8_t>(arSetupTime);

    /* t_clr = (TCLR + SET + 4) × THCLK
       t_clr / THCLK - 4 - SET  = TCLR */
    clrSetupTime = params[CHIP_PARAM_T_CLR] / tHCLK - 4 - stmParams->setupTime;
    clrSetupTime = clrSetupTime <= 0 ? 0 : ceil(clrSetupTime);
    stmParams->clrSetupTime = static_cast<uint8_t>(clrSetupTime);
}

const QByteArray &ParallelChipInfo::getHalConf()
{
    ParallelChipConf conf;
    StmParams stmParams;

    chipInfoToStmParams(&stmParams);

    conf.setupTime = stmParams.setupTime;
    conf.waitSetupTime = stmParams.waitSetupTime;
    conf.holdSetupTime = stmParams.holdSetupTime;
    conf.hiZSetupTime = stmParams.hiZSetupTime;
    conf.clrSetupTime = stmParams.clrSetupTime;
    conf.arSetupTime = stmParams.arSetupTime;
    conf.rowCycles = static_cast<uint8_t>(params[CHIP_PARAM_ROW_CYCLES]);
    conf.colCycles = static_cast<uint8_t>(params[CHIP_PARAM_COL_CYCLES]);
    conf.read1Cmd = static_cast<uint8_t>(params[CHIP_PARAM_READ1_CMD]);
    conf.read2Cmd = static_cast<uint8_t>(params[CHIP_PARAM_READ2_CMD]);
    conf.readSpareCmd = static_cast<uint8_t>(params[CHIP_PARAM_READ_SPARE_CMD]);
    conf.readIdCmd = static_cast<uint8_t>(params[CHIP_PARAM_READ_ID_CMD]);
    conf.resetCmd = static_cast<uint8_t>(params[CHIP_PARAM_RESET_CMD]);
    conf.write1Cmd = static_cast<uint8_t>(params[CHIP_PARAM_WRITE1_CMD]);
    conf.write2Cmd = static_cast<uint8_t>(params[CHIP_PARAM_WRITE2_CMD]);
    conf.erase1Cmd = static_cast<uint8_t>(params[CHIP_PARAM_ERASE1_CMD]);
    conf.erase2Cmd = static_cast<uint8_t>(params[CHIP_PARAM_ERASE2_CMD]);
    conf.statusCmd = static_cast<uint8_t>(params[CHIP_PARAM_STATUS_CMD]);
    conf.setFeaturesCmd = static_cast<uint8_t>(params[CHIP_PARAM_STATUS_CMD]);
    conf.enableEccAddr =
        static_cast<uint8_t>(params[CHIP_PARAM_ENABLE_HW_ECC_ADDR]);
    conf.enableEccValue =
        static_cast<uint8_t>(params[CHIP_PARAM_ENABLE_HW_ECC_VALUE]);
    conf.disableEccValue =
        static_cast<uint8_t>(params[CHIP_PARAM_DISABLE_HW_ECC_VALUE]);

    halConf.clear();
    halConf.append(reinterpret_cast<const char *>(&conf), sizeof(conf));

    return halConf;
}

quint64 ParallelChipInfo::getParam(uint32_t num)
{
    if (num >= CHIP_PARAM_NUM)
        return 0;

    return params[num];
}

int ParallelChipInfo::setParam(uint32_t num, quint64 value)
{
    if (num >= CHIP_PARAM_NUM)
        return -1;

    params[num] = value;

    return 0;
}

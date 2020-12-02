/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef PARALLEL_CHIP_INFO_H
#define PARALLEL_CHIP_INFO_H

#include "chip_info.h"

class ParallelChipInfo : public ChipInfo
{
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

    void chipInfoToStmParams(StmParams *stmParams);
public:
    ParallelChipInfo();
    virtual ~ParallelChipInfo();
    const QByteArray &getHalConf() override;
};

#endif // PARALLEL_CHIP_INFO_H

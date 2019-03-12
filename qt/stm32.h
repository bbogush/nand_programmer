#ifndef STM32_H
#define STM32_H

#include "chip_db.h"

typedef struct
{
    uint8_t setupTime;
    uint8_t waitSetupTime;
    uint8_t holdSetupTime;
    uint8_t hiZSetupTime;
    uint8_t clrSetupTime;
    uint8_t arSetupTime;
} StmParams;

void chipInfoToStmParams(ChipInfo *chipInfo, StmParams *stmParams);

#endif // STM32_H

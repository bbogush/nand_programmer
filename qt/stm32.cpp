#include "stm32.h"
#include <stdint.h>
#include <algorithm>
#include <array>
#include <math.h>

/* Convert NAND parameters to STM32 FSMC parameters.
 * See Application Note AN2784.
 */
void chipInfoToStmParams(ChipInfo *chipInfo, StmParams *stmParams)
{
    double setupTime, waitSetupTime, hiZSetupTime, holdSetupTime, arSetupTime,
        clrSetupTime;
    const double tHCLK = 13.88; /* 1 / 72MHz */
    const double tsuD_NOE = 25;
    std::array<uint32_t, 5> setupArr = { chipInfo->tCH, chipInfo->tCLS,
        chipInfo->tALS, chipInfo->tCLR, chipInfo->tAR };
    std::array<uint32_t, 3> hiZArr = { chipInfo->tCH, chipInfo->tALS,
        chipInfo->tCLS };
    std::array<uint32_t, 3> holdArr = { chipInfo->tCH, chipInfo->tCLH,
        chipInfo->tALH };

    /* (SET + 1) * tHCLK >= max(tCH, tCLS, tALS, tCLR, tAR) - tWP */
    setupTime = *std::max_element(setupArr.begin(), setupArr.end()) -
        chipInfo->tWP;
    setupTime = setupTime / tHCLK - 1;
    setupTime = setupTime <= 0 ? 0 : ceil(setupTime);
    stmParams->setupTime = static_cast<uint8_t>(setupTime);

    /* (WAIT + 1) * tHCLK >= max(tWP, tRP) */
    waitSetupTime = std::max(chipInfo->tWP, chipInfo->tRP);
    waitSetupTime = waitSetupTime / tHCLK - 1;
    waitSetupTime = waitSetupTime <= 0 ? 0 : ceil(waitSetupTime);
    stmParams->waitSetupTime = static_cast<uint8_t>(waitSetupTime);
    /* (WAIT + 1) *tHCLK >= tREA + tsuD_NOE */
    waitSetupTime = chipInfo->tREA + tsuD_NOE;
    waitSetupTime = waitSetupTime / tHCLK - 1;
    waitSetupTime = waitSetupTime <= 0 ? 0 : ceil(waitSetupTime);
    if (waitSetupTime > stmParams->waitSetupTime)
        stmParams->waitSetupTime = static_cast<uint8_t>(waitSetupTime);

    /* (HIZ + 1) * tHCLK >= max(tCH, tALS, tCLS) + (tWP - tDS) */
    hiZSetupTime = *std::max_element(hiZArr.begin(), hiZArr.end());
    hiZSetupTime -= chipInfo->tWP - chipInfo->tDS;
    hiZSetupTime = hiZSetupTime / tHCLK - 1;
    hiZSetupTime = hiZSetupTime <= 0 ? 0 : ceil(hiZSetupTime);
    stmParams->hiZSetupTime = static_cast<uint8_t>(hiZSetupTime);

    /* (HOLD + 1) * tHCLK >= max(tCH, tCLH, tALH) */
    holdSetupTime =*std::max_element(holdArr.begin(), holdArr.end());
    holdSetupTime = holdSetupTime / tHCLK - 1;
    /* K9F2G08U0C requires at least 1 tick */
    holdSetupTime = holdSetupTime <= 0 ? 1 : ceil(holdSetupTime);
    stmParams->holdSetupTime = static_cast<uint8_t>(holdSetupTime);

    /* ((WAIT + 1) + (HOLD + 1) + (SET + 1)) * tHCLK >= max(tWC, tRC) */
    while (((stmParams->setupTime + 1) + (stmParams->waitSetupTime + 1) +
        (stmParams->holdSetupTime + 1)) * tHCLK < std::max(chipInfo->tWC,
        chipInfo->tRC))
    {
        stmParams->setupTime++;
    }

    /* Not clear how to calculate, use the same approach as above */
    arSetupTime = chipInfo->tAR / tHCLK - 1;
    arSetupTime = arSetupTime <= 0 ? 0 : ceil(arSetupTime);
    stmParams->arSetupTime = static_cast<uint8_t>(arSetupTime);

    clrSetupTime = chipInfo->tCLR / tHCLK - 1;
    clrSetupTime = clrSetupTime <= 0 ? 0 : ceil(clrSetupTime);
    stmParams->clrSetupTime = static_cast<uint8_t>(clrSetupTime);
}

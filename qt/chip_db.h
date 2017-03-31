#ifndef CHIP_DB_H
#define CHIP_DB_H

#include <cstdint>

#define MAX_CHIP_NAME_LEN 16

enum
{
    CHIP_ID_NONE = 0,
    CHIP_ID_K9F2G08U0C = 1,
    CHIP_ID_LAST = 2,
};

typedef struct
{
    uint32_t num;
    char name[MAX_CHIP_NAME_LEN];
} ChipInfo;

uint32_t getChipDB(ChipInfo *&db);

#endif // CHIP_DB_H


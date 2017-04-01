#include "chip_db.h"

static ChipInfo chipDB[] =
{
    { CHIP_ID_NONE, "No Chip", 0 },
    { CHIP_ID_K9F2G08U0C, "K9F2G08U0C", 0x10000000 },
};

uint32_t getChipDB(ChipInfo *&db)
{
    db = chipDB;

    return CHIP_ID_LAST;
}

ChipInfo *getChiInfo(uint32_t id)
{
    if (id >= CHIP_ID_LAST)
        return 0;

    return &chipDB[id];
}

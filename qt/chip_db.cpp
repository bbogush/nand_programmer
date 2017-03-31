#include "chip_db.h"

static ChipInfo chipDB[] =
{
    { CHIP_ID_NONE,       "No Chip" },
    { CHIP_ID_K9F2G08U0C, "K9F2G08U0C" },
};

uint32_t getChipDB(ChipInfo *&db)
{
    db = chipDB;

    return CHIP_ID_LAST;
}

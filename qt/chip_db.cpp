#include "chip_db.h"
#include <cstring>

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

ChipInfo *getChipInfoByName(char *name)
{
    for (int id = 0; id < CHIP_ID_LAST; id++)
    {
        if (!strcmp(name, chipDB[id].name))
            return &chipDB[id];
    }

    return 0;
}

/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "chip_db.h"

chip_info_t chip_db[] =
{
    { CHIP_ID_NONE, "No Chip" },
    { CHIP_ID_K9F2G08U0C, "K9F2G08U0C", 0x800, 0x20000, 0x10000000, 1, 3, 1, 1, 1, 1 },
};

chip_info_t *chip_info_get(uint32_t chip_id)
{
    if (chip_id >= CHIP_ID_LAST)
        return 0;

    return &chip_db[chip_id];
}

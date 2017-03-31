/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef _CHIP_DB_H_
#define _CHIP_DB_H_

#define CHIP_NAME_LEN 15

typedef struct
{
    uint32_t id;
    char name[CHIP_NAME_LEN];
    uint32_t page_size;
    uint32_t block_size;
    uint32_t size;
    uint32_t setup_time;
    uint32_t wait_setup_time;
    uint32_t hold_setup_time;
    uint32_t hi_z_setup_time;
    uint32_t clr_setup_time;
    uint32_t ar_setup_time;
} chip_info_t;

enum
{
    CHIP_ID_NONE = 0,
    CHIP_ID_K9F2G08U0C = 1,
    CHIP_ID_LAST = 2,
};

chip_info_t *chip_info_selected_get();
int chip_select(uint32_t chip_id);
int chip_is_selected();

#endif /* _CHIP_DB_H_ */

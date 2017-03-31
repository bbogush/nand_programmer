/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef _CHIP_DB_H_
#define _CHIP_DB_H_

#define CHIP_NAME_LEN 15

typedef struct
{
    uint32_t num;
    char name[CHIP_NAME_LEN];
} chip_info_t;

enum
{
    CHIP_NUM_NONE = 0,
    CHIP_NUM_K9F2G08U0C = 1,
    CHIP_NUM_LAST = 2,
};

extern chip_info_t chip_db[];

#endif /* _CHIP_DB_H_ */

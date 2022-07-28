/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef _CHIP_INFO_H_
#define _CHIP_INFO_H_

typedef struct
{
    uint32_t page_size; /* without spare area */
    uint32_t block_size;
    uint64_t total_size;
    uint32_t spare_size;
    uint8_t bb_mark_off;
} chip_info_t;

#endif /* _CHIP_INFO_H_ */

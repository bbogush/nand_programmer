/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef _CHIP_INFO_H_
#define _CHIP_INFO_H_

typedef struct
{
    uint32_t page_size; /* without spare area */
    uint32_t block_size;
    uint32_t total_size;
    uint32_t spare_size;    
    uint8_t setup_time;
    uint8_t wait_setup_time;
    uint8_t hold_setup_time;
    uint8_t hi_z_setup_time;
    uint8_t clr_setup_time;
    uint8_t ar_setup_time;
    uint8_t row_cycles;
    uint8_t col_cycles;
} chip_info_t;

#endif /* _CHIP_INFO_H_ */

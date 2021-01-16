/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef _FLASH_HAL_H_
#define _FLASH_HAL_H_

#include "chip.h"
#include "chip_info.h"
#include <stdbool.h>

enum
{
    FLASH_STATUS_READY = 0,
    FLASH_STATUS_BUSY = 1,
    FLASH_STATUS_ERROR = 2,
    FLASH_STATUS_TIMEOUT = 3,
    FLASH_STATUS_INVALID_CMD = 4,
};

typedef struct
{
    int (*init)(void *conf, uint32_t conf_size);
    void (*uninit)();
    void (*reset)();
    void (*read_id)(chip_id_t *chip_id);
    void (*read_unique_id)(chip_unique_id_t *chip_uid);
    void (*set_ecc)(int i);
    uint32_t (*erase_block)(uint32_t page);
    uint32_t (*read_page)(uint8_t *buf, uint32_t page, uint32_t page_size,int ECC_status);
    uint32_t (*read_spare_data)(uint8_t *buf, uint32_t page, uint32_t offset,
        uint32_t data_size);
    void (*write_page_async)(uint8_t *buf, uint32_t page, uint32_t page_size);
    uint32_t (*read_status)();
    bool (*is_bb_supported)();
} flash_hal_t;

#endif /* _FLASH_HAL_H_ */

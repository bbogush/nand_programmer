/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef _SPI_FLASH_H_
#define _SPI_FLASH_H_

#include "chip.h"

void spi_flash_init();
void spi_flash_uninit();

void spi_flash_read_id(chip_id_t *chip_id);
void spi_flash_write_page_async(uint8_t *buf, uint32_t page,
    uint32_t page_size);
uint32_t spi_flash_write_page(uint8_t *buf, uint32_t page, uint32_t page_size);
uint32_t spi_flash_read_data(uint8_t *buf, uint32_t page, uint32_t page_offset,
    uint32_t data_size);
uint32_t spi_flash_read_page(uint8_t *buf, uint32_t page, uint32_t page_size);
uint32_t spi_flash_erase_block(uint32_t page);
uint32_t spi_flash_get_status();
uint32_t spi_flash_read_status();

#endif /* _SPI_FLASH_H_ */

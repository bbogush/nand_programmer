/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef _SPI_FLASH_H_
#define _SPI_FLASH_H_

#include "chip.h"

void spi_flash_init();
void spi_flash_uninit();

void spi_flash_read_id(chip_id_t *chip_id);

#endif /* _SPI_FLASH_H_ */

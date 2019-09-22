/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef _FSMC_NAND_H_
#define _FSMC_NAND_H_

#include "chip_info.h"

typedef struct
{
    uint8_t maker_id;
    uint8_t device_id;
    uint8_t third_id;
    uint8_t fourth_id;
} nand_id_t;

typedef struct 
{
    uint16_t zone;
    uint16_t block;
    uint16_t page;
} nand_addr_t;

#define CMD_AREA                   (uint32_t)(1<<16)  /* A16 = CLE  high */
#define ADDR_AREA                  (uint32_t)(1<<17)  /* A17 = ALE high */

#define DATA_AREA                  ((uint32_t)0x00000000) 

/* NAND memory status */  
#define NAND_VALID_ADDRESS         ((uint32_t)0x00000100)
#define NAND_INVALID_ADDRESS       ((uint32_t)0x00000200)
#define NAND_TIMEOUT_ERROR         ((uint32_t)0x00000400)
#define NAND_BUSY                  ((uint32_t)0x00000000)
#define NAND_ERROR                 ((uint32_t)0x00000001)
#define NAND_READY                 ((uint32_t)0x00000040)

/* FSMC NAND memory parameters */  
#define NAND_PAGE_SIZE             ((uint16_t)0x0800) /* 2048 bytes per page w/o Spare Area */
#define NAND_BLOCK_SIZE            ((uint16_t)0x0040) /* 64 pages per block */
#define NAND_ZONE_SIZE             ((uint16_t)0x0800) /* 2048 Block per zone (plane) */
#define NAND_SPARE_AREA_SIZE       ((uint16_t)0x0040) /* last 64 bytes as spare area */
#define NAND_MAX_ZONE              ((uint16_t)0x0001) /* 1 zones of 2048 block */

/* FSMC NAND memory address computation */  
#define ADDR_1st_CYCLE(ADDR)       (uint8_t)((ADDR)& 0xFF)               /* 1st addressing cycle */
#define ADDR_2nd_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF00) >> 8)      /* 2nd addressing cycle */
#define ADDR_3rd_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF0000) >> 16)   /* 3rd addressing cycle */
#define ADDR_4th_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF000000) >> 24) /* 4th addressing cycle */   

void nand_init(chip_info_t *chip_info);
void nand_read_id(nand_id_t *nand_id);
uint32_t nand_write_page(uint8_t *buf, uint32_t page, uint32_t page_size);
void nand_write_page_async(uint8_t *buf, uint32_t page, uint32_t page_size);
uint32_t nand_read_data(uint8_t *buf, uint32_t page, uint32_t page_offset,
    uint32_t data_size);
uint32_t nand_read_page(uint8_t *buf, uint32_t page, uint32_t page_size);
#if 0
uint32_t nand_write_spare_area(uint8_t *buf, nand_addr_t addr,
    uint32_t num_spare_area_to_write);
uint32_t nand_read_spare_area(uint8_t *buf, nand_addr_t addr,
    uint32_t num_spare_area_to_read);
#endif
uint32_t nand_erase_block(uint32_t page);
uint32_t nand_reset(void);
uint32_t nand_get_status(void);
uint32_t nand_read_status(void);
#if 0
uint32_t nand_addr_inc(nand_addr_t *addr);
uint32_t nand_raw_addr_to_nand_addr(uint32_t raw_addr, nand_addr_t *addr);
#endif

#endif /* _FSMC_NAND_H_ */

/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef _FSMC_NAND_H_
#define _FSMC_NAND_H_

#include "stm32f10x.h"

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

/* FSMC NAND memory command */  
#define NAND_CMD_AREA_A            ((uint8_t)0x00)
#define NAND_CMD_AREA_B            ((uint8_t)0x01)
#define NAND_CMD_AREA_C            ((uint8_t)0x50)

#define NAND_CMD_READ0             ((uint8_t)0x00)
#define NAND_CMD_READ1             ((uint8_t)0x30)

#define NAND_CMD_WRITE0            ((uint8_t)0x80)
#define NAND_CMD_WRITE_TRUE1       ((uint8_t)0x10)

#define NAND_CMD_ERASE0            ((uint8_t)0x60)
#define NAND_CMD_ERASE1            ((uint8_t)0xD0)

#define NAND_CMD_READID            ((uint8_t)0x90)
#define NAND_CMD_STATUS            ((uint8_t)0x70)
#define NAND_CMD_LOCK_STATUS       ((uint8_t)0x7A)
#define NAND_CMD_RESET             ((uint8_t)0xFF)

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

void nand_init(uint32_t chip_id);
void nand_read_id(nand_id_t *nand_id);
uint32_t nand_write_small_page(uint8_t *buf, nand_addr_t addr,
    uint32_t num_pages_to_write);
uint32_t nand_read_small_page(uint8_t *buf, nand_addr_t addr,
    uint32_t num_page_to_read);
uint32_t nand_write_spare_area(uint8_t *buf, nand_addr_t addr,
    uint32_t num_spare_area_to_write);
uint32_t nand_read_spare_area(uint8_t *buf, nand_addr_t addr,
    uint32_t num_spare_area_to_read);
uint32_t nand_erase_block(nand_addr_t addr);
uint32_t nand_reset(void);
uint32_t nand_get_status(void);
uint32_t nand_read_status(void);
uint32_t nand_addr_inc(nand_addr_t *addr);
uint32_t nand_raw_addr_to_nand_addr(uint32_t raw_addr, nand_addr_t *addr);

#endif /* _FSMC_NAND_H_ */

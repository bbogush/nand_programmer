/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "fsmc_nand.h"
#include "log.h"
#include <stm32f10x.h>

#define CMD_AREA                   (uint32_t)(1<<16)  /* A16 = CLE  high */
#define ADDR_AREA                  (uint32_t)(1<<17)  /* A17 = ALE high */

#define DATA_AREA                  ((uint32_t)0x00000000) 

/* NAND memory status */  
#define NAND_ERROR                 ((uint32_t)0x00000001)
#define NAND_READY                 ((uint32_t)0x00000040)

/* FSMC NAND memory address computation */
/* 1st addressing cycle */
#define ADDR_1st_CYCLE(ADDR)       (uint8_t)((ADDR)& 0xFF)
/* 2nd addressing cycle */
#define ADDR_2nd_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF00) >> 8)
/* 3rd addressing cycle */
#define ADDR_3rd_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF0000) >> 16)
/* 4th addressing cycle */
#define ADDR_4th_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF000000) >> 24)

#define FSMC_Bank_NAND     FSMC_Bank2_NAND
#define Bank_NAND_ADDR     Bank2_NAND_ADDR 
#define Bank2_NAND_ADDR    ((uint32_t)0x70000000)     
#define ROW_ADDRESS (addr.page + (addr.block + (addr.zone * NAND_ZONE_SIZE)) * \
    NAND_BLOCK_SIZE)

#define UNDEFINED_CMD 0xFF

typedef struct __attribute__((__packed__))
{
    uint8_t setup_time;
    uint8_t wait_setup_time;
    uint8_t hold_setup_time;
    uint8_t hi_z_setup_time;
    uint8_t clr_setup_time;
    uint8_t ar_setup_time;
    uint8_t row_cycles;
    uint8_t col_cycles;
    uint8_t read1_cmd;
    uint8_t read2_cmd;
    uint8_t read_spare_cmd;
    uint8_t read_id_cmd;
    uint8_t reset_cmd;
    uint8_t write1_cmd;
    uint8_t write2_cmd;
    uint8_t erase1_cmd;
    uint8_t erase2_cmd;
    uint8_t status_cmd;
    uint8_t set_features_cmd;
    uint8_t enable_ecc_addr;
    uint8_t enable_ecc_value;
    uint8_t disable_ecc_value;
} fsmc_conf_t;

static fsmc_conf_t fsmc_conf;

static void nand_gpio_init(void)
{
    GPIO_InitTypeDef gpio_init;
  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | 
        RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG, ENABLE);
  
    /* CLE, ALE, D0->D3, NOE, NWE and NCE2 NAND pin configuration */
    gpio_init.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_14 | GPIO_Pin_15 |
        GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &gpio_init);

    /* D4->D7 NAND pin configuration */  
    gpio_init.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_Init(GPIOE, &gpio_init);

    /* NWAIT NAND pin configuration */
    gpio_init.GPIO_Pin = GPIO_Pin_6;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOD, &gpio_init); 

    /* INT2 NAND pin configuration, not available in LQFP100 */
#if 0
    gpio_init.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOG, &gpio_init);
#endif

}

static void nand_fsmc_init()
{
    FSMC_NANDInitTypeDef fsmc_init;
    FSMC_NAND_PCCARDTimingInitTypeDef timing_init;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

    timing_init.FSMC_SetupTime = fsmc_conf.setup_time;
    timing_init.FSMC_WaitSetupTime = fsmc_conf.wait_setup_time;
    timing_init.FSMC_HoldSetupTime = fsmc_conf.hold_setup_time;
    timing_init.FSMC_HiZSetupTime = fsmc_conf.hi_z_setup_time;

    fsmc_init.FSMC_Bank = FSMC_Bank2_NAND;
    fsmc_init.FSMC_Waitfeature = FSMC_Waitfeature_Enable;
    fsmc_init.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_8b;
    fsmc_init.FSMC_ECC = FSMC_ECC_Enable;
    fsmc_init.FSMC_ECCPageSize = FSMC_ECCPageSize_2048Bytes;
    fsmc_init.FSMC_TCLRSetupTime = fsmc_conf.clr_setup_time;
    fsmc_init.FSMC_TARSetupTime = fsmc_conf.ar_setup_time;
    fsmc_init.FSMC_CommonSpaceTimingStruct = &timing_init;
    fsmc_init.FSMC_AttributeSpaceTimingStruct = &timing_init;
    FSMC_NANDInit(&fsmc_init);

    FSMC_NANDCmd(FSMC_Bank2_NAND, ENABLE);
}

static void nand_print_fsmc_info()
{
    DEBUG_PRINT("Setup time: %d\r\n", fsmc_conf.setup_time);
    DEBUG_PRINT("Wait setup time: %d\r\n", fsmc_conf.wait_setup_time);
    DEBUG_PRINT("Hold setup time: %d\r\n", fsmc_conf.hold_setup_time);
    DEBUG_PRINT("HiZ setup time: %d\r\n", fsmc_conf.hi_z_setup_time);
    DEBUG_PRINT("CLR setip time: %d\r\n", fsmc_conf.clr_setup_time);
    DEBUG_PRINT("AR setip time: %d\r\n", fsmc_conf.ar_setup_time);
    DEBUG_PRINT("Row cycles: %d\r\n", fsmc_conf.row_cycles);
    DEBUG_PRINT("Col. cycles: %d\r\n", fsmc_conf.col_cycles);
    DEBUG_PRINT("Read command 1: %d\r\n", fsmc_conf.read1_cmd);
    DEBUG_PRINT("Read command 2: %d\r\n", fsmc_conf.read2_cmd);
    DEBUG_PRINT("Read spare command: %d\r\n", fsmc_conf.read_spare_cmd);    
    DEBUG_PRINT("Read ID command: %d\r\n", fsmc_conf.read_id_cmd);
    DEBUG_PRINT("Reset command: %d\r\n", fsmc_conf.reset_cmd);
    DEBUG_PRINT("Write 1 command: %d\r\n", fsmc_conf.write1_cmd);
    DEBUG_PRINT("Write 2 command: %d\r\n", fsmc_conf.write2_cmd);
    DEBUG_PRINT("Erase 1 command: %d\r\n", fsmc_conf.erase1_cmd);
    DEBUG_PRINT("Erase 2 command: %d\r\n", fsmc_conf.erase2_cmd);
    DEBUG_PRINT("Status command: %d\r\n", fsmc_conf.status_cmd);
//    DEBUG_PRINT("Set feature command: %d\r\n", fsmc_conf.set_feature_cmd);
    DEBUG_PRINT("Enable ECC address: %d\r\n", fsmc_conf.enable_ecc_addr);
    DEBUG_PRINT("Enable ECC value: %d\r\n", fsmc_conf.enable_ecc_value);
    DEBUG_PRINT("Disable ECC value: %d\r\n", fsmc_conf.disable_ecc_value);
}

static void nand_reset()
{
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.reset_cmd;
}

static int nand_init(void *conf, uint32_t conf_size)
{
    if (conf_size < sizeof(fsmc_conf_t))
        return -1;
   
    fsmc_conf = *(fsmc_conf_t *)conf;

    nand_gpio_init();
    nand_fsmc_init(fsmc_conf);
    nand_print_fsmc_info();
    nand_reset();

    return 0;
}

static void nand_uninit()
{
    //TODO
}

static uint32_t nand_read_status()
{
    uint32_t data, status;

    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.status_cmd;
    data = *(__IO uint8_t *)(Bank_NAND_ADDR);

    if ((data & NAND_ERROR) == NAND_ERROR)
        status = FLASH_STATUS_ERROR;
    else if ((data & NAND_READY) == NAND_READY)
        status = FLASH_STATUS_READY;
    else
        status = FLASH_STATUS_BUSY;

    return status;
}

static uint32_t nand_get_status()
{
    uint32_t status, timeout = 0x1000000;

    status = nand_read_status();

    /* Wait for a NAND operation to complete or a TIMEOUT to occur */
    while (status == FLASH_STATUS_BUSY && timeout)
    {
        status = nand_read_status();
        timeout --;
    }

    if (!timeout)
        status =  FLASH_STATUS_TIMEOUT;

    return status;
}

static void nand_read_id(chip_id_t *nand_id)
{
    uint32_t data = 0;

    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.read_id_cmd;
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;

    /* Sequence to read ID from NAND flash */
    data = *(__IO uint32_t *)(Bank_NAND_ADDR | DATA_AREA);
    nand_id->maker_id   = ADDR_1st_CYCLE(data);
    nand_id->device_id  = ADDR_2nd_CYCLE(data);
    nand_id->third_id   = ADDR_3rd_CYCLE(data);
    nand_id->fourth_id  = ADDR_4th_CYCLE(data);

    data = *((__IO uint32_t *)(Bank_NAND_ADDR | DATA_AREA) + 1);
    nand_id->fifth_id   = ADDR_1st_CYCLE(data);
}

static void nand_write_page_async(uint8_t *buf, uint32_t page,
    uint32_t page_size)
{
    uint32_t i;

    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.write1_cmd;

    switch (fsmc_conf.col_cycles)
    {
    case 1:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        break;
    case 2:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        break;
    case 3:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        break;
    case 4:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        break;
    default:
        break;
    }

    switch (fsmc_conf.row_cycles)
    {
    case 1:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        break;
    case 2:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);
        break;
    case 3:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(page);
        break;
    case 4:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_4th_CYCLE(page);
        break;
    default:
        break;
    }

    for(i = 0; i < page_size; i++)
        *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA) = buf[i];

    if (fsmc_conf.write2_cmd != UNDEFINED_CMD)
        *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.write2_cmd;
}

static uint32_t nand_read_data(uint8_t *buf, uint32_t page,
    uint32_t page_offset, uint32_t data_size)
{
    uint32_t i;

    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.read1_cmd;

    switch (fsmc_conf.col_cycles)
    {
    case 1:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_1st_CYCLE(page_offset);
        break;
    case 2:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_1st_CYCLE(page_offset);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_2nd_CYCLE(page_offset);
        break;
    case 3:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_1st_CYCLE(page_offset);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_2nd_CYCLE(page_offset);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_3rd_CYCLE(page_offset);
        break;
    case 4:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_1st_CYCLE(page_offset);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_2nd_CYCLE(page_offset);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_3rd_CYCLE(page_offset);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_4th_CYCLE(page_offset);
    default:
        break;
    }

    switch (fsmc_conf.row_cycles)
    {
    case 1:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        break;
    case 2:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);
        break;
    case 3:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(page);
        break;
    case 4:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_4th_CYCLE(page);
        break;
    default:
        break;
    }

    if (fsmc_conf.read2_cmd != UNDEFINED_CMD)
        *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.read2_cmd;

    for (i = 0; i < data_size; i++)
        buf[i] = *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA);

    return nand_get_status();
}

static uint32_t nand_read_page(uint8_t *buf, uint32_t page, uint32_t page_size)
{
    return nand_read_data(buf, page, 0, page_size);
}

static uint32_t nand_read_spare_data(uint8_t *buf, uint32_t page,
    uint32_t offset, uint32_t data_size)
{
    uint32_t i;

    if (fsmc_conf.read_spare_cmd == UNDEFINED_CMD)
        return FLASH_STATUS_INVALID_CMD;

    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.read_spare_cmd;

    switch (fsmc_conf.col_cycles)
    {
    case 1:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_1st_CYCLE(offset);
        break;
    case 2:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_1st_CYCLE(offset);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_2nd_CYCLE(offset);
        break;
    case 3:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_1st_CYCLE(offset);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_2nd_CYCLE(offset);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_3rd_CYCLE(offset);
        break;
    case 4:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_1st_CYCLE(offset);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_2nd_CYCLE(offset);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_3rd_CYCLE(offset);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_4th_CYCLE(offset);
    default:
        break;
    }

    switch (fsmc_conf.row_cycles)
    {
    case 1:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        break;
    case 2:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);
        break;
    case 3:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(page);
        break;
    case 4:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_4th_CYCLE(page);
        break;
    default:
        break;
    }

    for (i = 0; i < data_size; i++)
        buf[i] = *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA);

    return nand_get_status();
}

static uint32_t nand_erase_block(uint32_t page)
{
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.erase1_cmd;

    switch (fsmc_conf.row_cycles)
    {
    case 1:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        break;
    case 2:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);
        break;
    case 3:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(page);
        break;
    case 4:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_4th_CYCLE(page);
        break;
    default:
        break;
    }

    if (fsmc_conf.erase2_cmd != UNDEFINED_CMD)
        *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.erase2_cmd;

    return nand_get_status();
}

static inline bool nand_is_bb_supported()
{
    return true;
}

static uint32_t nand_enable_hw_ecc(bool enable)
{
    uint8_t enable_ecc;

    if (fsmc_conf.set_features_cmd == UNDEFINED_CMD)
        return FLASH_STATUS_INVALID_CMD;

    enable_ecc = enable ? fsmc_conf.enable_ecc_value :
        fsmc_conf.disable_ecc_value;

    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.set_features_cmd;
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = fsmc_conf.enable_ecc_addr;
    *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA) = enable_ecc;
    *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA) = 0;
    *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA) = 0;
    *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA) = 0;

    return 0;
}

flash_hal_t hal_fsmc =
{
    .init = nand_init,
    .uninit = nand_uninit,
    .read_id = nand_read_id,
    .erase_block = nand_erase_block,
    .read_page = nand_read_page,
    .read_spare_data = nand_read_spare_data,
    .write_page_async = nand_write_page_async,
    .read_status = nand_read_status,
    .is_bb_supported = nand_is_bb_supported,
    .enable_hw_ecc = nand_enable_hw_ecc,
};


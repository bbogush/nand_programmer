/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "fsmc_nand.h"
#include <stm32f10x.h>

#define FSMC_Bank_NAND     FSMC_Bank2_NAND
#define Bank_NAND_ADDR     Bank2_NAND_ADDR 
#define Bank2_NAND_ADDR    ((uint32_t)0x70000000)     
#define ROW_ADDRESS (addr.page + (addr.block + (addr.zone * NAND_ZONE_SIZE)) * \
    NAND_BLOCK_SIZE)

#define UNDEFINED_CMD 0xFF

typedef struct
{
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
} fsmc_cmd_t;

static fsmc_cmd_t fsmc_cmd;

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

static void nand_fsmc_init(chip_info_t *chip_info)
{
    FSMC_NANDInitTypeDef fsmc_init;
    FSMC_NAND_PCCARDTimingInitTypeDef timing_init;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

    timing_init.FSMC_SetupTime = chip_info->setup_time;
    timing_init.FSMC_WaitSetupTime = chip_info->wait_setup_time;
    timing_init.FSMC_HoldSetupTime = chip_info->hold_setup_time;
    timing_init.FSMC_HiZSetupTime = chip_info->hi_z_setup_time;

    fsmc_init.FSMC_Bank = FSMC_Bank2_NAND;
    fsmc_init.FSMC_Waitfeature = FSMC_Waitfeature_Enable;
    fsmc_init.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_8b;
    fsmc_init.FSMC_ECC = FSMC_ECC_Enable;
    fsmc_init.FSMC_ECCPageSize = FSMC_ECCPageSize_2048Bytes;
    fsmc_init.FSMC_TCLRSetupTime = chip_info->clr_setup_time;
    fsmc_init.FSMC_TARSetupTime = chip_info->ar_setup_time;
    fsmc_init.FSMC_CommonSpaceTimingStruct = &timing_init;
    fsmc_init.FSMC_AttributeSpaceTimingStruct = &timing_init;
    FSMC_NANDInit(&fsmc_init);

    FSMC_NANDCmd(FSMC_Bank2_NAND, ENABLE);
}

static void nand_cmd_init(chip_info_t *chip_info)
{
    fsmc_cmd.row_cycles = chip_info->row_cycles;
    fsmc_cmd.col_cycles = chip_info->col_cycles;
    fsmc_cmd.read1_cmd = chip_info->read1_cmd;
    fsmc_cmd.read2_cmd = chip_info->read2_cmd;
    fsmc_cmd.read_spare_cmd = chip_info->read_spare_cmd;
    fsmc_cmd.read_id_cmd = chip_info->read_id_cmd;
    fsmc_cmd.reset_cmd = chip_info->reset_cmd;
    fsmc_cmd.write1_cmd = chip_info->write1_cmd;
    fsmc_cmd.write2_cmd = chip_info->write2_cmd;
    fsmc_cmd.erase1_cmd = chip_info->erase1_cmd;
    fsmc_cmd.erase2_cmd = chip_info->erase2_cmd;
    fsmc_cmd.status_cmd = chip_info->status_cmd;
}

void nand_init(chip_info_t *chip_info)
{
    nand_gpio_init();
    nand_fsmc_init(chip_info);
    nand_cmd_init(chip_info);
}

void nand_read_id(nand_id_t *nand_id)
{
    uint32_t data = 0;

    /* Send Command to the command area */
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_cmd.read_id_cmd;
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;

    /* Sequence to read ID from NAND flash */
    data = *(__IO uint32_t *)(Bank_NAND_ADDR | DATA_AREA);

    nand_id->maker_id   = ADDR_1st_CYCLE (data);
    nand_id->device_id  = ADDR_2nd_CYCLE (data);
    nand_id->third_id   = ADDR_3rd_CYCLE (data);
    nand_id->fourth_id  = ADDR_4th_CYCLE (data);
}

void nand_write_page_async(uint8_t *buf, uint32_t page, uint32_t page_size)
{
    uint32_t i;

    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_cmd.write1_cmd;

    switch (fsmc_cmd.col_cycles)
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

    switch (fsmc_cmd.row_cycles)
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

    if (fsmc_cmd.write2_cmd != UNDEFINED_CMD)
        *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_cmd.write2_cmd;
}

uint32_t nand_write_page(uint8_t *buf, uint32_t page, uint32_t page_size)
{
    nand_write_page_async(buf, page, page_size);
 
    return nand_get_status();
}

uint32_t nand_read_data(uint8_t *buf, uint32_t page, uint32_t page_offset,
    uint32_t data_size)
{
    uint32_t i;

    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_cmd.read1_cmd;

    switch (fsmc_cmd.col_cycles)
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

    switch (fsmc_cmd.row_cycles)
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

    if (fsmc_cmd.read2_cmd != UNDEFINED_CMD)
        *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_cmd.read2_cmd;

    for (i = 0; i < data_size; i++)
        buf[i]= *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA);

    return nand_get_status();
}

uint32_t nand_read_page(uint8_t *buf, uint32_t page, uint32_t page_size)
{
    return nand_read_data(buf, page, 0, page_size);
}

uint32_t nand_read_spare_data(uint8_t *buf, uint32_t page, uint32_t offset,
    uint32_t data_size)
{
    uint32_t i;

    if (fsmc_cmd.read_spare_cmd == UNDEFINED_CMD)
        return NAND_INVALID_CMD;

    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_cmd.read_spare_cmd;

    switch (fsmc_cmd.col_cycles)
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

    switch (fsmc_cmd.row_cycles)
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

#if 0
/**
  * @brief  This routine write the spare area information for the specified
  *         pages addresses.  
  * @param  buf: pointer on the Buffer containing data to be written 
  * @param  addr: First page address
  * @param  num_spare_area_to_write: Number of Spare Area to write
  * @retval New status of the NAND operation. This parameter can be:
  *          - NAND_TIMEOUT_ERROR: when the previous operation generate 
  *          a Timeout error
  *          - NAND_READY: when memory is ready for the next operation 
  *            And the new status of the increment address operation. It can be:
  *          - NAND_VALID_ADDRESS: When the new address is valid address
  *          - NAND_INVALID_ADDRESS: When the new address is invalid address
  */
uint32_t nand_write_spare_area(uint8_t *buf, nand_addr_t addr,
    uint32_t num_spare_area_to_write)
{
    uint32_t index = 0x00, num_spare_area_written = 0x00,
        address_status = NAND_VALID_ADDRESS;
    uint32_t status = NAND_READY, size = 0x00; 

    while ((num_spare_area_to_write != 0x00) &&
        (address_status == NAND_VALID_ADDRESS) && (status == NAND_READY))
    {
        /* Page write Spare area command and address */
        *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_AREA_C;
        *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_WRITE0;

        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00; 
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_1st_CYCLE(ROW_ADDRESS);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_2nd_CYCLE(ROW_ADDRESS);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_3rd_CYCLE(ROW_ADDRESS);

        /* Calculate the size */ 
        size = NAND_SPARE_AREA_SIZE + (NAND_SPARE_AREA_SIZE *
            num_spare_area_written);

        /* Write the data */ 
        for (; index < size; index++)
            *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA) = buf[index];
    
        *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_WRITE_TRUE1;

        /* Check status for successful operation */
        status = nand_get_status();

        if (status == NAND_READY)
        {
            num_spare_area_written++;

            num_spare_area_to_write--;
    
            /* Calculate Next page Address */
            address_status = nand_addr_inc(&addr);
        }
    }
  
    return status | address_status;
}

/**
  * @brief  This routine read the spare area information from the specified
  *         pages addresses.  
  * @param  buf: pointer on the Buffer to fill 
  * @param  addr: First page address
  * @param  num_spare_area_to_read: Number of Spare Area to read
  * @retval New status of the NAND operation. This parameter can be:
  *          - NAND_TIMEOUT_ERROR: when the previous operation generate 
  *            a Timeout error
  *          - NAND_READY: when memory is ready for the next operation 
  *            And the new status of the increment address operation. It can be:
  *          - NAND_VALID_ADDRESS: When the new address is valid address
  *          - NAND_INVALID_ADDRESS: When the new address is invalid address
  */
uint32_t nand_read_spare_area(uint8_t *buf, nand_addr_t addr,
    uint32_t num_spare_area_to_read)
{
    uint32_t num_spare_area_read = 0x00, index = 0x00,
        address_status = NAND_VALID_ADDRESS;
    uint32_t status = NAND_READY, size = 0x00;

    while ((num_spare_area_to_read != 0x0) &&
        (address_status == NAND_VALID_ADDRESS))
    {     
        /* Page Read command and page address */
        *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_AREA_C;

        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00; 
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_1st_CYCLE(ROW_ADDRESS);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_2nd_CYCLE(ROW_ADDRESS);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_3rd_CYCLE(ROW_ADDRESS);

        /* Data Read */
        size = NAND_SPARE_AREA_SIZE + (NAND_SPARE_AREA_SIZE *
            num_spare_area_read);

        /* Get Data into Buffer */
        for ( ;index < size; index++)
            buf[index] = *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA);
    
        num_spare_area_read++;
    
        num_spare_area_to_read--;

        /* Calculate page address */
        address_status = nand_addr_inc(&addr);
    }

    status = nand_get_status();

    return status | address_status;
}
#endif

uint32_t nand_erase_block(uint32_t page)
{
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_cmd.erase1_cmd;

    switch (fsmc_cmd.row_cycles)
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

    if (fsmc_cmd.erase2_cmd != UNDEFINED_CMD)
        *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_cmd.erase2_cmd;

    return nand_get_status();
}

/**
  * @brief  This routine reset the NAND FLASH.
  * @param  None
  * @retval NAND_READY
  */
uint32_t nand_reset()
{
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_cmd.reset_cmd;

    return (NAND_READY);
}

/**
  * @brief  Get the NAND operation status.
  * @param  None
  * @retval New status of the NAND operation. This parameter can be:
  *          - NAND_TIMEOUT_ERROR: when the previous operation generate
  *            a Timeout error
  *          - NAND_READY: when memory is ready for the next operation
  */
uint32_t nand_get_status(void)
{
    uint32_t timeout = 0x1000000, status = NAND_READY;

    status = nand_read_status();

    /* Wait for a NAND operation to complete or a TIMEOUT to occur */
    while (status == NAND_BUSY && timeout)
    {
        status = nand_read_status();
        timeout --;
    }

    if (!timeout)
        status =  NAND_TIMEOUT_ERROR;

    /* Return the operation status */
    return (status);
}

/**
  * @brief  Reads the NAND memory status using the Read status command. 
  * @param  None
  * @retval The status of the NAND memory. This parameter can be:
  *          - NAND_BUSY: when memory is busy
  *          - NAND_READY: when memory is ready for the next operation
  *          - NAND_ERROR: when the previous operation gererates error
  */
uint32_t nand_read_status(void)
{
    uint32_t data = 0x00, status = NAND_BUSY;

    /* Read status operation */
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_cmd.status_cmd;
    data = *(__IO uint8_t *)(Bank_NAND_ADDR);

    if ((data & NAND_ERROR) == NAND_ERROR)
        status = NAND_ERROR;
    else if((data & NAND_READY) == NAND_READY)
        status = NAND_READY;
    else
        status = NAND_BUSY;

    return status;
}

#if 0
/**
  * @brief  Increment the NAND memory address. 
  * @param  addr: address to be incremented.
  * @retval The new status of the increment address operation. It can be:
  *          - NAND_VALID_ADDRESS: When the new address is valid address
  *          - NAND_INVALID_ADDRESS: When the new address is invalid address   
  */
uint32_t nand_addr_inc(nand_addr_t *addr)
{
    uint32_t status = NAND_VALID_ADDRESS;
 
    addr->page++;

    if (addr->page == NAND_BLOCK_SIZE)
    {
        addr->page = 0;
        addr->block++;

        if (addr->block == NAND_ZONE_SIZE)
        {
            addr->block = 0;
            addr->zone++;

            if (addr->zone == NAND_MAX_ZONE)
                status = NAND_INVALID_ADDRESS;
        }
    }

    return status;
}

uint32_t nand_raw_addr_to_nand_addr(uint32_t raw_addr, nand_addr_t *addr)
{
    uint32_t blocks_in_zones, block_size;
    uint32_t status = NAND_VALID_ADDRESS;

    block_size = NAND_BLOCK_SIZE * NAND_PAGE_SIZE;
    addr->zone = raw_addr / (NAND_ZONE_SIZE * block_size);
    blocks_in_zones = addr->zone * NAND_ZONE_SIZE;
    addr->block = raw_addr / block_size - blocks_in_zones;
    addr->page = raw_addr / NAND_PAGE_SIZE - (blocks_in_zones +
       addr->block) * NAND_BLOCK_SIZE;

    if(addr->zone == NAND_MAX_ZONE)
        status = NAND_INVALID_ADDRESS;

    return status;
}

#endif

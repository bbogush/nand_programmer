/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "fsmc_nand.h"
#include "chip_db.h"

#define FSMC_Bank_NAND     FSMC_Bank2_NAND
#define Bank_NAND_ADDR     Bank2_NAND_ADDR 
#define Bank2_NAND_ADDR    ((uint32_t)0x70000000)     
#define ROW_ADDRESS (addr.page + (addr.block + (addr.zone * NAND_ZONE_SIZE)) * \
    NAND_BLOCK_SIZE)

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

void nand_init(uint32_t chip_id)
{
    FSMC_NANDInitTypeDef fsmc_init;
    FSMC_NAND_PCCARDTimingInitTypeDef timing_init;
    chip_info_t *chip_info = chip_info_get(chip_id);

    nand_gpio_init();

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);    

    /*-- FSMC Configuration --------------------------------------------------*/

    /* Calculations of timing paramaters
     *           _   _   _   _   _   _   _   _   _   _   _   _
     * HCLK    _| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |
     *         _____ _________________________________ ________
     * A[25:0] _____|_________________________________|________ 
     *         _____                                   ________
     * NCE          |_________________________________|        
     *
     *              |SET+1|      WAIT+1       |HOLD+1 |        
     *         ___________                     ________________
     * NOE/NWE            |___________________|                
     *
     *              | HIZ+1 |
     * Write                 _________________________         
     * data    -------------|_________________________|--------
     * 
     * Read                __________________________
     * data    -----------|__________________________|--------- 
     *
     * (SET + 1) * tHCLK >= max(tCS, tCLS, tALS) - tWP
     * (SET + 1) * tHCLK >= max(tCLR, tAR)
     * (WAIT + 1) * tHCLK >= max(tWP, tRP)
     * (WAIT + 1) * tHCLK >= (tREA + tsu(D-NOE))   
     * (HIZ + 1) * tHCLK >= max(tCS , tALS, tCLS) - tDS
     * (HOLD + 1) x tHCLK >= max(tCH, tCLH, tALH)
     * ((WAIT + 1) + (HOLD + 1) + (SET + 1)) x tHCLK >= max(tWC, tRC)
     * tsu(D-NOE) = 25ns
     * tHCLK = 1/72MHz = 13.89ns
     */

    /* K9F2G08U0C parameters:
     * tCS = 20ns
     * tCLS = 12ns
     * tALS = 12ns
     * tCLR = 10ns
     * tAR = 10ns
     * tWP = 12ns
     * tRP = 12ns
     * tDS = 12ns
     * tCH = 5ns
     * tCLH = 5ns
     * tALH = 5ns
     * tWC = 25ns
     * tRC = 25ns
     * tREA = 20ns
     */

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

void nand_read_id(nand_id_t *nand_id)
{
    uint32_t data = 0;

    /* Send Command to the command area */
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = 0x90;
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;

    /* Sequence to read ID from NAND flash */
    data = *(__IO uint32_t *)(Bank_NAND_ADDR | DATA_AREA);

    nand_id->maker_id   = ADDR_1st_CYCLE (data);
    nand_id->device_id  = ADDR_2nd_CYCLE (data);
    nand_id->third_id   = ADDR_3rd_CYCLE (data);
    nand_id->fourth_id  = ADDR_4th_CYCLE (data);
}

/**
  * @brief  This routine is for writing one or several 512 Bytes Page size.
  * @param  buf: pointer on the Buffer containing data to be written 
  * @param  addr: First page address
  * @param  num_pages_to_write: Number of page to write  
  * @retval New status of the NAND operation. This parameter can be:
  *          - NAND_TIMEOUT_ERROR: when the previous operation generate 
  *            a Timeout error
  *          - NAND_READY: when memory is ready for the next operation 
  *            And the new status of the increment address operation. It can be:
  *          - NAND_VALID_ADDRESS: When the new address is valid address
  *          - NAND_INVALID_ADDRESS: When the new address is invalid address  
  */
uint32_t nand_write_small_page(uint8_t *buf, nand_addr_t addr,
    uint32_t num_pages_to_write)
{
    uint32_t index = 0x00, num_pages_written = 0x00,
        address_status = NAND_VALID_ADDRESS;
    uint32_t status = NAND_READY, size = 0x00;

    while ((num_pages_to_write != 0x00) &&
        (address_status == NAND_VALID_ADDRESS) && (status == NAND_READY))
    {
        /* Page write command and address */
        *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_WRITE0;

        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_1st_CYCLE(ROW_ADDRESS);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_2nd_CYCLE(ROW_ADDRESS);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_3rd_CYCLE(ROW_ADDRESS);

        /* Calculate the size */
        size = NAND_PAGE_SIZE + (NAND_PAGE_SIZE * num_pages_written);

        /* Write data */
        for(; index < size; index++)
            *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA) = buf[index];
    
        *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_WRITE_TRUE1;

        /* Check status for successful operation */
        status = nand_get_status();
    
        if (status == NAND_READY)
        {
            num_pages_written++;

            num_pages_to_write--;

            /* Calculate Next small page Address */
            address_status = nand_addr_inc(&addr);
        }
    }
  
    return status | address_status;
}

/**
  * @brief  This routine is for sequential read from one or several 512 Bytes
  *         Page size.
  * @param  buf: pointer on the Buffer to fill
  * @param  addr: First page address
  * @param  num_pages_to_read: Number of page to read  
  * @retval New status of the NAND operation. This parameter can be:
  *          - NAND_TIMEOUT_ERROR: when the previous operation generate 
  *            a Timeout error
  *          - NAND_READY: when memory is ready for the next operation 
  *            And the new status of the increment address operation. It can be:
  *          - NAND_VALID_ADDRESS: When the new address is valid address
  *          - NAND_INVALID_ADDRESS: When the new address is invalid address
  */
uint32_t nand_read_small_page(uint8_t *buf, nand_addr_t addr,
    uint32_t num_pages_to_read)
{
    uint32_t index = 0x00, num_pages_read = 0x00,
        address_status = NAND_VALID_ADDRESS;
    uint32_t status = NAND_READY, size = 0x00;

    while ((num_pages_to_read != 0x0) && (address_status == NAND_VALID_ADDRESS))
    {
        /* Page Read command and page address */
        *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_READ0;
   
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00; 
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_1st_CYCLE(ROW_ADDRESS);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_2nd_CYCLE(ROW_ADDRESS);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) =
            ADDR_3rd_CYCLE(ROW_ADDRESS);

        *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_READ1;
    
        /* Calculate the size */
        size = NAND_PAGE_SIZE + (NAND_PAGE_SIZE * num_pages_read);
    
        /* Get Data into Buffer */    
        for (; index < size; index++)
            buf[index]= *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA);

        num_pages_read++;

        num_pages_to_read--;

        /* Calculate page address */
        address_status = nand_addr_inc(&addr);
    }

    status = nand_get_status();
  
    return status | address_status;
}

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

/**
  * @brief  This routine erase complete block from NAND FLASH
  * @param  addr: Any address into block to be erased
  * @retval New status of the NAND operation. This parameter can be:
  *          - NAND_TIMEOUT_ERROR: when the previous operation generate 
  *            a Timeout error
  *          - NAND_READY: when memory is ready for the next operation 
  */
uint32_t nand_erase_block(nand_addr_t addr)
{
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_ERASE0;

    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(ROW_ADDRESS);
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(ROW_ADDRESS);
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(ROW_ADDRESS);

    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_ERASE1; 

    return nand_get_status();
}

/**
  * @brief  This routine reset the NAND FLASH.
  * @param  None
  * @retval NAND_READY
  */
uint32_t nand_reset(void)
{
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_RESET;

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
    while ((status != NAND_READY) && (timeout != 0x00))
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
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_STATUS;
    data = *(__IO uint8_t *)(Bank_NAND_ADDR);

    if ((data & NAND_ERROR) == NAND_ERROR)
        status = NAND_ERROR;
    else if((data & NAND_READY) == NAND_READY)
        status = NAND_READY;
    else
        status = NAND_BUSY;

    return status;
}

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

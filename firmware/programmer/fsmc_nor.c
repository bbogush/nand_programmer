//
// Created by Kunkka on 13/11/24.
//

#include "fsmc_nor.h"
#include "log.h"
#include <stm32f10x.h>

#define CMD_AREA                   (uint32_t)(1<<16)  /* A16 = CLE  high - Not used for NOR */
#define ADDR_AREA                  (uint32_t)(1<<17)  /* A17 = ALE high */

#define DATA_AREA                  ((uint32_t)0x00000000) 

/* NOR flash status */
#define NOR_ERROR                  ((uint32_t)0x00000001)
#define NOR_READY                  ((uint32_t)0x00000040)

/* FSMC NOR memory address computation - simplified for NOR */
#define ADDR_1st_CYCLE(ADDR)       (uint8_t)((ADDR)& 0xFF)
#define ADDR_2nd_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF00) >> 8)
#define ADDR_3rd_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF0000) >> 16)
#define ADDR_4th_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF000000) >> 24)

#define FSMC_Bank_NOR     FSMC_Bank1_NORSRAM1  // Use Bank1 for NOR
#define Bank_NOR_ADDR      ((uint32_t)0x60000000)  // Bank 1 NOR flash address (commonly used)
#define ROW_ADDRESS (addr.page + (addr.block + (addr.zone * NOR_ZONE_SIZE)) * \
    NOR_BLOCK_SIZE)

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
    uint8_t read_cmd;      // Read command for NOR flash (example: 0xFF for generic read)
    uint8_t write_cmd;     // Write command for NOR flash (example: 0xA0 for generic write)
    uint8_t erase_cmd;     // Erase command for NOR flash (example: 0xD0 for sector erase)
    uint8_t status_cmd;    // Status command for NOR flash
    uint8_t read_id_cmd;   // Command for reading ID (usually 0x90 for NOR)
} fsmc_conf_t;

static fsmc_conf_t fsmc_conf;

static void nor_gpio_init(void)
{
    GPIO_InitTypeDef gpio_init;
  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | 
        RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG, ENABLE);
  
    /* Address and Data lines (D0 to D15) for NOR flash */
    gpio_init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 |
                         GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 |
                         GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 |
                         GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &gpio_init);

    /* Control lines (WE, RE, OE, and CE) for NOR flash */
    gpio_init.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_Init(GPIOE, &gpio_init);

    /* Ready/Busy pin (if applicable) */
    gpio_init.GPIO_Pin = GPIO_Pin_6;  // Optional: check datasheet for specific pin
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOD, &gpio_init); 
}

static void nor_fsmc_init(void)
{
    FSMC_NORSRAMInitTypeDef fsmc_init;
    FSMC_NORSRAMTimingInitTypeDef timing_init;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

    // Set timing parameters specific to NOR flash (adjust these according to the datasheet)
    timing_init.FSMC_SetupTime = 1;   // Adjust setup time for NOR flash
    timing_init.FSMC_WaitSetupTime = 1; // Adjust wait time for NOR flash
    timing_init.FSMC_HoldSetupTime = 1; // Adjust hold time for NOR flash
    timing_init.FSMC_HiZSetupTime = 1; // High impedance time (if necessary)

    fsmc_init.FSMC_Bank = FSMC_Bank1_NORSRAM1;  // Use FSMC Bank 1 for NOR flash
    fsmc_init.FSMC_Waitfeature = FSMC_Waitfeature_Disable; // Typically disabled for NOR flash
    fsmc_init.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b; // 16-bit data width
    fsmc_init.FSMC_ECC = FSMC_ECC_Disable;  // ECC typically not used for NOR flash
    fsmc_init.FSMC_ECCPageSize = FSMC_ECCPageSize_2048Bytes;  // Default ECC page size
    fsmc_init.FSMC_TCLRSetupTime = 1;   // Adjust based on NOR flash datasheet
    fsmc_init.FSMC_TARSetupTime = 1;    // Adjust based on NOR flash datasheet
    fsmc_init.FSMC_CommonSpaceTimingStruct = &timing_init;
    fsmc_init.FSMC_AttributeSpaceTimingStruct = &timing_init;

    // Initialize FSMC for NOR flash
    FSMC_NORSRAMInit(&fsmc_init);

    // Enable FSMC for NOR flash access
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);
}

// Reading a 16-bit data from NOR flash at a specific address
uint16_t nor_flash_read(uint32_t address)
{
    return *((volatile uint16_t*) (Bank_NOR_ADDR + address));  // Direct memory-mapped access
}

// Writing a 16-bit data to NOR flash at a specific address
void nor_flash_write(uint32_t address, uint16_t data)
{
    *((volatile uint16_t*) (Bank_NOR_ADDR + address)) = data;  // Direct memory-mapped access
}


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

typedef struct __attribute__((__packed__)) {
    uint8_t address_setup_time;        // Setup time for address to data transfer
    uint8_t address_hold_time;         // Hold time for address after data transfer
    uint8_t data_setup_time;           // Data setup time (after address)
    uint8_t bus_turnaround_duration;   // Bus turnaround duration (between read/write)
    uint8_t clk_division;              // Clock division factor for FSMC operations
    uint8_t data_latency;              // Data latency (before data transfer starts)
    uint8_t access_mode;

    // Command sequences
    uint8_t read1_cmd;          // First read command (usually 0x00 for NOR)
    uint8_t read2_cmd;          // Second read command (if applicable)
    uint8_t write1_cmd;         // First write command (usually 0x40 for NOR)
    uint8_t write2_cmd;         // Second write command (if applicable)

    uint8_t reset_cmd;          // Reset command (if applicable)
    uint8_t erase_cmd;          // Erase command (if applicable)
    uint8_t status_cmd;         // Status command (for checking flash status)
    uint8_t read_id_cmd;   // Command for reading ID (usually 0x90 for NOR)
    uint8_t enable_ecc_value;   // ECC enable value (only applicable to some NOR flashes)
    uint8_t disable_ecc_value;  // ECC disable value (only applicable to some NOR flashes)

    uint8_t row_cycles;         // Number of row address cycles (typically 1 or 2 for NOR)
    uint8_t col_cycles;         // Number of column address cycles (typically 1 or 2 for NOR)
} fsmc_conf_t;


static fsmc_conf_t fsmc_conf;

static void nor_gpio_init(void) {
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

static void nor_fsmc_init(void) {
    FSMC_NORSRAMInitTypeDef fsmc_init;
    FSMC_NORSRAMTimingInitTypeDef timing_init;

    timing_init.FSMC_AddressSetupTime = fsmc_conf.address_setup_time;
    timing_init.FSMC_DataSetupTime = fsmc_conf.data_setup_time;  // Data setup time
    timing_init.FSMC_AddressHoldTime = fsmc_conf.address_hold_time;
    timing_init.FSMC_BusTurnAroundDuration = fsmc_conf.bus_turnaround_duration;  // Bus turnaround duration
    timing_init.FSMC_CLKDivision = fsmc_conf.clk_division;      // FSMC clock division
    timing_init.FSMC_DataLatency = fsmc_conf.data_latency;      // Data latency
    timing_init.FSMC_AccessMode = fsmc_conf.access_mode;        // Access mode

    // Enable the FSMC peripheral clock
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

    fsmc_init.FSMC_Bank = FSMC_Bank1_NORSRAM1;  // NOR flash on Bank 1
    fsmc_init.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b; // 16-bit data width
    fsmc_init.FSMC_DataAddressMux = FSMC_DataAddressMux_Enable;
    fsmc_init.FSMC_MemoryType = FSMC_MemoryType_SRAM;
    fsmc_init.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
    fsmc_init.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
    fsmc_init.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
    fsmc_init.FSMC_WrapMode = FSMC_WrapMode_Disable;
    fsmc_init.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
    fsmc_init.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
    fsmc_init.FSMC_WaitSignal = FSMC_WaitSignal_Enable;
    fsmc_init.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
    fsmc_init.FSMC_WriteBurst = FSMC_WriteBurst_Disable;

    fsmc_init.FSMC_ReadWriteTimingStruct = &timing_init;
    fsmc_init.FSMC_WriteTimingStruct = &timing_init;
//    fsmc_init->FSMC_ReadWriteTimingStruct->FSMC_AddressSetupTime = 0xF;
//    fsmc_init->FSMC_ReadWriteTimingStruct->FSMC_AddressHoldTime = 0xF;
//    fsmc_init->FSMC_ReadWriteTimingStruct->FSMC_DataSetupTime = 0xFF;
//    fsmc_init->FSMC_ReadWriteTimingStruct->FSMC_BusTurnAroundDuration = 0xF;
//    fsmc_init->FSMC_ReadWriteTimingStruct->FSMC_CLKDivision = 0xF;
//    fsmc_init->FSMC_ReadWriteTimingStruct->FSMC_DataLatency = 0xF;
//    fsmc_init->FSMC_ReadWriteTimingStruct->FSMC_AccessMode = FSMC_AccessMode_A;
//    fsmc_init->FSMC_WriteTimingStruct->FSMC_AddressSetupTime = 0xF;
//    fsmc_init->FSMC_WriteTimingStruct->FSMC_AddressHoldTime = 0xF;
//    fsmc_init->FSMC_WriteTimingStruct->FSMC_DataSetupTime = 0xFF;
//    fsmc_init->FSMC_WriteTimingStruct->FSMC_BusTurnAroundDuration = 0xF;
//    fsmc_init->FSMC_WriteTimingStruct->FSMC_CLKDivision = 0xF;
//    fsmc_init->FSMC_WriteTimingStruct->FSMC_DataLatency = 0xF;
//    fsmc_init->FSMC_WriteTimingStruct->FSMC_AccessMode = FSMC_AccessMode_A;

// Initialize FSMC for NOR Flash
    FSMC_NORSRAMInit(&fsmc_init);

// Enable FSMC for NOR flash access
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);
}


// Reading a 16-bit data from NOR flash at a specific address
uint16_t nor_flash_read(uint32_t address) {
    return *((volatile uint16_t *) (Bank_NOR_ADDR + address));  // Direct memory-mapped access
}

// Writing a 16-bit data to NOR flash at a specific address
void nor_flash_write(uint32_t address, uint16_t data) {
    *((volatile uint16_t *) (Bank_NOR_ADDR + address)) = data;  // Direct memory-mapped access
}

static uint32_t nor_flash_read_spare_data(uint8_t *buf, uint32_t page, uint32_t offset, uint32_t data_size) {
    // Since NOR flash doesn't have spare data, return invalid command error
    return FLASH_STATUS_INVALID_CMD;
}

static uint32_t nor_flash_read_page(uint8_t *buf, uint32_t page, uint32_t page_size) {
    // Read the page from NOR flash (example implementation)
    // The address could be calculated using the page number
    uint32_t i = 0;
    for (i = 0; i < page_size; i++) {
        buf[i] = *((volatile uint8_t *)(Bank_NOR_ADDR + (page * page_size) + i));
    }
    return FLASH_STATUS_READY;
}

static void nor_flash_write_page_async(uint8_t *buf, uint32_t page, uint32_t page_size) {
    // Example asynchronous write (using polling or interrupts)
    uint32_t i = 0;
    for (i = 0; i < page_size; i++) {
        *((volatile uint8_t *) (Bank_NOR_ADDR + (page * page_size) + i)) = buf[i];
    }
}

static uint32_t nor_flash_read_status() {
    // Return the flash status (dummy implementation, typically status registers)
    return FLASH_STATUS_READY;
}

static bool nor_flash_is_bb_supported() {
    // NOR flash doesn't support bad block management
    return false;
}

static uint32_t nor_flash_enable_hw_ecc(bool enable) {
    // NOR flash typically doesn't require ECC, but we can simulate it
    if (enable) {
        // Enable ECC (not applicable for most NOR flash chips)
        return FLASH_STATUS_READY;
    }
    // Disable ECC
    return FLASH_STATUS_READY;
}

static void nor_print_fsmc_info() {
    // Printing out the FSMC configuration values for the NOR flash
    DEBUG_PRINT("FSMC Configuration Information for NOR Flash:\r\n");

    DEBUG_PRINT("Setup time: %d\r\n", fsmc_conf.setup_time);
    DEBUG_PRINT("Wait setup time: %d\r\n", fsmc_conf.wait_setup_time);
    DEBUG_PRINT("Hold setup time: %d\r\n", fsmc_conf.hold_setup_time);
    DEBUG_PRINT("High-Z setup time: %d\r\n", fsmc_conf.hi_z_setup_time);
    DEBUG_PRINT("Clear setup time: %d\r\n", fsmc_conf.clr_setup_time);
    DEBUG_PRINT("Address setup time: %d\r\n", fsmc_conf.ar_setup_time);

    DEBUG_PRINT("Read Command 1: 0x%02X\r\n", fsmc_conf.read1_cmd);  // Print first read command
    DEBUG_PRINT("Read Command 2: 0x%02X\r\n", fsmc_conf.read2_cmd);  // Print second read command

    DEBUG_PRINT("Write Command: 0x%02X\r\n", fsmc_conf.write_cmd);
    DEBUG_PRINT("Reset Command: 0x%02X\r\n", fsmc_conf.reset_cmd);
    DEBUG_PRINT("Erase Command: 0x%02X\r\n", fsmc_conf.erase_cmd);
    DEBUG_PRINT("Status Command: 0x%02X\r\n", fsmc_conf.status_cmd);

    DEBUG_PRINT("Row address cycles: %d\r\n", fsmc_conf.row_cycles);
    DEBUG_PRINT("Column address cycles: %d\r\n", fsmc_conf.col_cycles);
}

static void nor_flash_reset(void) {
    // Send the reset command to the NOR flash chip via FSMC
    *(__IO
    uint8_t *)(Bank_NOR_ADDR | CMD_AREA) = fsmc_conf.reset_cmd;

    DEBUG_PRINT("NOR flash reset complete.\r\n");
}

// Initialize NOR Flash
static int nor_flash_init(void *conf, uint32_t conf_size) {
    nor_gpio_init();
    nor_fsmc_init();
    nor_print_fsmc_info();
    nor_flash_reset();
    return FLASH_STATUS_READY;
}

// Uninitialize NOR Flash
static void nor_flash_uninit() {
    // Disable FSMC or other NOR flash-specific configurations
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, DISABLE);
}

// Read NOR Flash ID
static void nor_flash_read_id(chip_id_t *nor_id) {
    uint32_t data = 0;

    // Send the read ID command to the NOR flash
    *(__IO
    uint8_t *)(Bank_NOR_ADDR | CMD_AREA) = fsmc_conf.read_id_cmd;

    // Send the address for the read ID command (usually 0x00 or a specific address depending on the chip)
    *(__IO
    uint8_t *)(Bank_NOR_ADDR | ADDR_AREA) = 0x00;

    // Read the 32-bit ID value from the NOR flash
    data = *(__IO
    uint32_t *)(Bank_NOR_ADDR | DATA_AREA);

    // Extract the ID parts from the 32-bit data
    nor_id->maker_id = ADDR_1st_CYCLE(data);  // Manufacturer ID (usually 1 byte)
    nor_id->device_id = ADDR_2nd_CYCLE(data);  // Device ID (usually 1 byte)
    nor_id->third_id = ADDR_3rd_CYCLE(data);  // Third ID (if applicable)
    nor_id->fourth_id = ADDR_4th_CYCLE(data);  // Fourth ID (if applicable)

    // Optionally, you may want to read more ID data if the NOR flash provides more than 4 parts
    // Here we read additional data if applicable, depending on your flash type
    data = *((__IO
    uint32_t *)(Bank_NOR_ADDR | DATA_AREA) + 1);

    // You may have additional ID fields to extract if necessary
    nor_id->fifth_id = ADDR_1st_CYCLE(data);  // Fifth ID (if available)
}


flash_hal_t hal_fsmc_nor = {
        .init = nor_flash_init,
        .uninit = nor_flash_uninit,
        .read_id = nor_flash_read_id,
        .erase_block = NULL,  // NOR Flash typically doesn’t use block erase like NAND
        .read_page = nor_flash_read_page,
        .read_spare_data = nor_flash_read_spare_data,  // Unsupported for NOR Flash
        .write_page_async = nor_flash_write_page_async,
        .read_status = nor_flash_read_status,
        .is_bb_supported = nor_flash_is_bb_supported,
        .enable_hw_ecc = nor_flash_enable_hw_ecc,
};

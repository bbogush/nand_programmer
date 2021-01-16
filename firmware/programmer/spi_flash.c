/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "spi_flash.h"
#include <stm32f10x.h>

#define SPI_FLASH_CS_PIN GPIO_Pin_4
#define SPI_FLASH_SCK_PIN GPIO_Pin_5
#define SPI_FLASH_MISO_PIN GPIO_Pin_6
#define SPI_FLASH_MOSI_PIN GPIO_Pin_7

#define FLASH_DUMMY_BYTE 0xA5

#define FLASH_READY 0
#define FLASH_BUSY  1
#define FLASH_TIMEOUT 2

/* 1st addressing cycle */
#define ADDR_1st_CYCLE(ADDR) (uint8_t)((ADDR)& 0xFF)
/* 2nd addressing cycle */
#define ADDR_2nd_CYCLE(ADDR) (uint8_t)(((ADDR)& 0xFF00) >> 8)
/* 3rd addressing cycle */
#define ADDR_3rd_CYCLE(ADDR) (uint8_t)(((ADDR)& 0xFF0000) >> 16)
/* 4th addressing cycle */
#define ADDR_4th_CYCLE(ADDR) (uint8_t)(((ADDR)& 0xFF000000) >> 24)

#define UNDEFINED_CMD 0xFF

typedef struct __attribute__((__packed__))
{
    uint8_t page_offset;
    uint8_t read_cmd;
    uint8_t read_id_cmd;
    uint8_t write_cmd;
    uint8_t write_en_cmd;
    uint8_t erase_cmd;
    uint8_t status_cmd;
    uint8_t busy_bit;
    uint8_t busy_state;
    uint32_t freq;
} spi_conf_t;

static spi_conf_t spi_conf;

static void spi_flash_gpio_init()
{
    GPIO_InitTypeDef gpio_init;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* Enable SPI peripheral clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  
    /* Configure SPI SCK pin */
    gpio_init.GPIO_Pin = SPI_FLASH_SCK_PIN;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpio_init);

    /* Configure SPI MOSI pin */
    gpio_init.GPIO_Pin = SPI_FLASH_MOSI_PIN;
    GPIO_Init(GPIOA, &gpio_init);

    /* Configure SPI MISO pin */
    gpio_init.GPIO_Pin = SPI_FLASH_MISO_PIN;
    gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio_init);
  
    /* Configure SPI CS pin */
    gpio_init.GPIO_Pin = SPI_FLASH_CS_PIN;
    gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &gpio_init);
}

static void spi_flash_gpio_uninit()
{
    GPIO_InitTypeDef gpio_init;

    /* Disable SPI peripheral clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, DISABLE);

    /* Disable SPI SCK pin */
    gpio_init.GPIO_Pin = SPI_FLASH_SCK_PIN;
    gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio_init);

    /* Disable SPI MISO pin */
    gpio_init.GPIO_Pin = SPI_FLASH_MISO_PIN;
    GPIO_Init(GPIOA, &gpio_init);

    /* Disable SPI MOSI pin */
    gpio_init.GPIO_Pin = SPI_FLASH_MOSI_PIN;
    GPIO_Init(GPIOA, &gpio_init);

    /* Disable SPI CS pin */
    gpio_init.GPIO_Pin = SPI_FLASH_CS_PIN;
    GPIO_Init(GPIOA, &gpio_init);
}

static inline void spi_flash_select_chip()
{
    GPIO_ResetBits(GPIOA, SPI_FLASH_CS_PIN);
}

static inline void spi_flash_deselect_chip()
{
    GPIO_SetBits(GPIOA, SPI_FLASH_CS_PIN);
}

static uint16_t spi_flash_get_baud_rate_prescaler(uint32_t spi_freq_khz)
{
    uint32_t system_clock_khz = SystemCoreClock / 1000;

    if (spi_freq_khz >= system_clock_khz / 2)
        return SPI_BaudRatePrescaler_2;
    else if (spi_freq_khz >= system_clock_khz / 4)
        return SPI_BaudRatePrescaler_4;
    else if (spi_freq_khz >= system_clock_khz / 8)
        return SPI_BaudRatePrescaler_8;
    else if (spi_freq_khz >= system_clock_khz / 16)
        return SPI_BaudRatePrescaler_16;
    else if (spi_freq_khz >= system_clock_khz / 32)
        return SPI_BaudRatePrescaler_32;
    else if (spi_freq_khz >= system_clock_khz / 64)
        return SPI_BaudRatePrescaler_64;
    else if (spi_freq_khz >= system_clock_khz / 128)
        return SPI_BaudRatePrescaler_128;
    else
        return SPI_BaudRatePrescaler_256;
}

static int spi_flash_init(void *conf, uint32_t conf_size)
{
    SPI_InitTypeDef spi_init;

    if (conf_size < sizeof(spi_conf_t))
        return -1; 
    spi_conf = *(spi_conf_t *)conf;

    spi_flash_gpio_init();

    spi_flash_deselect_chip();

    /* Configure SPI */
    spi_init.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi_init.SPI_Mode = SPI_Mode_Master;
    spi_init.SPI_DataSize = SPI_DataSize_8b;
    spi_init.SPI_CPOL = SPI_CPOL_High;
    spi_init.SPI_CPHA = SPI_CPHA_2Edge;
    spi_init.SPI_NSS = SPI_NSS_Soft;
    spi_init.SPI_BaudRatePrescaler =
        spi_flash_get_baud_rate_prescaler(spi_conf.freq);
    spi_init.SPI_FirstBit = SPI_FirstBit_MSB;
    spi_init.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &spi_init);

    /* Enable SPI */
    SPI_Cmd(SPI1, ENABLE);

    return 0;
}

static void spi_flash_uninit()
{
    spi_flash_gpio_uninit();

    /* Disable SPI */
    SPI_Cmd(SPI1, DISABLE);
}

static uint8_t spi_flash_send_byte(uint8_t byte)
{
    /* Loop while DR register in not emplty */
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

    /* Send byte through the SPI1 peripheral to generate clock signal */
    SPI_I2S_SendData(SPI1, byte);

    /* Wait to receive a byte */
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

    /* Return the byte read from the SPI bus */
    return SPI_I2S_ReceiveData(SPI1);
}

static inline uint8_t spi_flash_read_byte()
{
    return spi_flash_send_byte(FLASH_DUMMY_BYTE);
}

static uint32_t spi_flash_read_status()
{
    uint8_t status;
    uint32_t flash_status = FLASH_READY;

    spi_flash_select_chip();

    spi_flash_send_byte(spi_conf.status_cmd);

    status = spi_flash_read_byte();

    if (spi_conf.busy_state == 1 && (status & (1 << spi_conf.busy_bit)))
        flash_status = FLASH_BUSY;
    else if (spi_conf.busy_state == 0 && !(status & (1 << spi_conf.busy_bit)))
        flash_status = FLASH_BUSY;

    spi_flash_deselect_chip();

    return flash_status;
}

static uint32_t spi_flash_get_status()
{
    uint32_t status, timeout = 0x1000000;

    status = spi_flash_read_status();

    /* Wait for an operation to complete or a TIMEOUT to occur */
    while (status == FLASH_BUSY && timeout)
    {
        status = spi_flash_read_status();
        timeout --;
    }

    if (!timeout)
        status = FLASH_TIMEOUT;

    return status;
}

static void spi_flash_read_id(chip_id_t *chip_id)
{
    spi_flash_select_chip();

    spi_flash_send_byte(spi_conf.read_id_cmd);

    chip_id->maker_id = spi_flash_read_byte();
    chip_id->device_id = spi_flash_read_byte();
    chip_id->third_id = spi_flash_read_byte();
    chip_id->fourth_id = spi_flash_read_byte();

    spi_flash_deselect_chip();
}

static void spi_flash_write_enable()
{
    if (spi_conf.write_en_cmd == UNDEFINED_CMD)
        return;

    spi_flash_select_chip();
    spi_flash_send_byte(spi_conf.write_en_cmd);
    spi_flash_deselect_chip();
}

static void spi_flash_write_page_async(uint8_t *buf, uint32_t page,
    uint32_t page_size)
{
    uint32_t i;

    spi_flash_write_enable();

    spi_flash_select_chip();

    spi_flash_send_byte(spi_conf.write_cmd);

    page = page << spi_conf.page_offset;

    spi_flash_send_byte(ADDR_3rd_CYCLE(page));
    spi_flash_send_byte(ADDR_2nd_CYCLE(page));
    spi_flash_send_byte(ADDR_1st_CYCLE(page));

    for (i = 0; i < page_size; i++)
        spi_flash_send_byte(buf[i]);

    spi_flash_deselect_chip();
}

static uint32_t spi_flash_read_data(uint8_t *buf, uint32_t page,
    uint32_t page_offset, uint32_t data_size)
{
    uint32_t i, addr = (page << spi_conf.page_offset) + page_offset;

    spi_flash_select_chip();

    spi_flash_send_byte(spi_conf.read_cmd);

    spi_flash_send_byte(ADDR_3rd_CYCLE(addr));
    spi_flash_send_byte(ADDR_2nd_CYCLE(addr));
    spi_flash_send_byte(ADDR_1st_CYCLE(addr));

    /* AT45DB requires write of dummy byte after address */
    spi_flash_send_byte(FLASH_DUMMY_BYTE);

    for (i = 0; i < data_size; i++)
        buf[i] = spi_flash_read_byte();

    spi_flash_deselect_chip();

    return FLASH_READY;
}

static uint32_t spi_flash_read_page(uint8_t *buf, uint32_t page,
    uint32_t page_size,int ecc_enabled)
{
    return spi_flash_read_data(buf, page, 0, page_size);
}

static uint32_t spi_flash_read_spare_data(uint8_t *buf, uint32_t page,
    uint32_t offset, uint32_t data_size)
{
    return FLASH_STATUS_INVALID_CMD;
}

static uint32_t spi_flash_erase_block(uint32_t page)
{
    uint32_t addr = page << spi_conf.page_offset;

    spi_flash_write_enable();

    spi_flash_select_chip();

    spi_flash_send_byte(spi_conf.erase_cmd);

    spi_flash_send_byte(ADDR_3rd_CYCLE(addr));
    spi_flash_send_byte(ADDR_2nd_CYCLE(addr));
    spi_flash_send_byte(ADDR_1st_CYCLE(addr));

    spi_flash_deselect_chip();

    return spi_flash_get_status();
}

static inline bool spi_flash_is_bb_supported()
{
    return false;
}

flash_hal_t hal_spi =
{
    .init = spi_flash_init,
    .uninit = spi_flash_uninit,
    .read_id = spi_flash_read_id,
    .erase_block = spi_flash_erase_block,
    .read_page = spi_flash_read_page,
    .read_spare_data = spi_flash_read_spare_data, 
    .write_page_async = spi_flash_write_page_async,
    .read_status = spi_flash_read_status,
    .is_bb_supported = spi_flash_is_bb_supported
};

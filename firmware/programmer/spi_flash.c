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

#define CMD_READ_ID 0x9F
#define CMD_FLASH_PAGE_WRITE 0x82
#define CMD_FLASH_PAGE_READ 0x0B
#define CMD_FLASH_BLOCK_ERASE 0x50
#define CMD_READ_STATUS 0xD7

#define PAGE_ADDRESS_OFFSET 9
#define BLOCK_ADDRESS_OFFSET 12

#define STATUS_READY (1<<7)

#define FLASH_READY 0
#define FLASH_BUSY  1
#define FLASH_TIMEOUT 2

/* 1st addressing cycle */
#define ADDR_1st_CYCLE(ADDR) (uint8_t)((ADDR)& 0xFF)
/* 2st addressing cycle */
#define ADDR_2nd_CYCLE(ADDR) (uint8_t)(((ADDR)& 0xFF00) >> 8)
/* 3st addressing cycle */
#define ADDR_3rd_CYCLE(ADDR) (uint8_t)(((ADDR)& 0xFF0000) >> 16)
/* 4st addressing cycle */
#define ADDR_4th_CYCLE(ADDR) (uint8_t)(((ADDR)& 0xFF000000) >> 24)

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

void spi_flash_init()
{
    SPI_InitTypeDef spi_init;

    spi_flash_gpio_init();

    spi_flash_deselect_chip();

    /* Configure SPI */
    spi_init.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi_init.SPI_Mode = SPI_Mode_Master;
    spi_init.SPI_DataSize = SPI_DataSize_8b;
    spi_init.SPI_CPOL = SPI_CPOL_High;
    spi_init.SPI_CPHA = SPI_CPHA_2Edge;
    spi_init.SPI_NSS = SPI_NSS_Soft;
    spi_init.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    spi_init.SPI_FirstBit = SPI_FirstBit_MSB;
    spi_init.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &spi_init);

    /* Enable SPI */
    SPI_Cmd(SPI1, ENABLE);
}

void spi_flash_uninit()
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

void spi_flash_read_id(chip_id_t *chip_id)
{
    spi_flash_select_chip();

    spi_flash_send_byte(CMD_READ_ID);

    chip_id->maker_id = spi_flash_read_byte();
    chip_id->device_id = spi_flash_read_byte();
    chip_id->third_id = spi_flash_read_byte();
    chip_id->fourth_id = spi_flash_read_byte();

    spi_flash_deselect_chip();
}

void spi_flash_write_page_async(uint8_t *buf, uint32_t page, uint32_t page_size)
{
    uint32_t i;

    spi_flash_select_chip();

    spi_flash_send_byte(CMD_FLASH_PAGE_WRITE);

    page = page << PAGE_ADDRESS_OFFSET;

    spi_flash_send_byte(ADDR_1st_CYCLE(page));
    spi_flash_send_byte(ADDR_2nd_CYCLE(page));
    spi_flash_send_byte(ADDR_3rd_CYCLE(page));

    for (i = 0; i < page_size; i++)
        spi_flash_send_byte(buf[i]);

    spi_flash_deselect_chip();
}

uint32_t spi_flash_write_page(uint8_t *buf, uint32_t page, uint32_t page_size)
{
    spi_flash_write_page_async(buf, page, page_size);
 
    return spi_flash_get_status();
}

uint32_t spi_flash_read_data(uint8_t *buf, uint32_t page, uint32_t page_offset,
    uint32_t data_size)
{
    uint32_t i, addr = (page << PAGE_ADDRESS_OFFSET) + page_offset;

    spi_flash_select_chip();

    spi_flash_send_byte(CMD_FLASH_PAGE_READ);

    spi_flash_send_byte(ADDR_1st_CYCLE(addr));
    spi_flash_send_byte(ADDR_2nd_CYCLE(addr));
    spi_flash_send_byte(ADDR_3rd_CYCLE(addr));

    /* AT45DB requires write of dummy byte after address */
    spi_flash_send_byte(FLASH_DUMMY_BYTE);

    for (i = 0; i < data_size; i++)
        buf[i] = spi_flash_read_byte();

    spi_flash_deselect_chip();

    return FLASH_READY;
}

uint32_t spi_flash_read_page(uint8_t *buf, uint32_t page, uint32_t page_size)
{
    return spi_flash_read_data(buf, page, 0, page_size);
}

uint32_t spi_flash_erase_block(uint32_t page)
{
    uint32_t addr = page << BLOCK_ADDRESS_OFFSET;
    spi_flash_select_chip();

    spi_flash_send_byte(CMD_FLASH_BLOCK_ERASE);

    spi_flash_send_byte(ADDR_1st_CYCLE(addr));
    spi_flash_send_byte(ADDR_2nd_CYCLE(addr));
    spi_flash_send_byte(ADDR_3rd_CYCLE(addr));

    spi_flash_deselect_chip();

    return spi_flash_get_status();
}

uint32_t spi_flash_get_status()
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


uint32_t spi_flash_read_status()
{
    uint32_t status;

    spi_flash_select_chip();

    spi_flash_send_byte(CMD_READ_STATUS);

    if (spi_flash_read_byte() & STATUS_READY)
        status = FLASH_READY;
    else
        status = FLASH_BUSY;

    spi_flash_deselect_chip();

    return status;
}

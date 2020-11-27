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
#define READ_ID_CMD 0x9F

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

void sip_flash_deselect_chip()
{
    GPIO_SetBits(GPIOA, SPI_FLASH_CS_PIN);
}

void sip_flash_select_chip()
{
    GPIO_ResetBits(GPIOA, SPI_FLASH_CS_PIN);
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

void spi_flash_init()
{
    SPI_InitTypeDef spi_init;

    spi_flash_gpio_init();

    sip_flash_deselect_chip();

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

void spi_flash_read_id(chip_id_t *chip_id)
{
    sip_flash_select_chip();

    spi_flash_send_byte(READ_ID_CMD);

    chip_id->maker_id = spi_flash_send_byte(FLASH_DUMMY_BYTE);
    chip_id->device_id = spi_flash_send_byte(FLASH_DUMMY_BYTE);
    chip_id->third_id = spi_flash_send_byte(FLASH_DUMMY_BYTE);
    chip_id->fourth_id = spi_flash_send_byte(FLASH_DUMMY_BYTE);

    sip_flash_select_chip();
}

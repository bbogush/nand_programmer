/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "uart.h"
#include "version.h"
#include <stdio.h>
#include <string.h>

/* Flash layout
 * ------------ 0x08000000
 * |bootloader |
 * |14K        |
 * -------------0x08003800
 * |data       |
 * |2K         |
 * ------------ 0x08004000
 * |image1     |   
 * |120K       |
 * -------------0x08022000
 * |image2     |
 * |120K       |
 * -------------0x08040000
 */

#define STR(a) #a
#define MAKE_STR(a) STR(a)
#define VERSION "\r\nBootloader ver: " MAKE_STR(SW_VERSION_MAJOR) "." \
    MAKE_STR(SW_VERSION_MINOR) "." MAKE_STR(SW_VERSION_BUILD) "\r\n"

#define APP1_ADDRESS_OFFSET 0x4000
#define APP1_ADDRESS (FLASH_BASE + APP1_ADDRESS_OFFSET)
#define APP2_ADDRESS_OFFSET 0x22000
#define APP2_ADDRESS (FLASH_BASE + APP2_ADDRESS_OFFSET)

typedef void (*app_func_t)(void);
typedef struct __attribute__((__packed__))
{
    uint8_t active_image;
} config_t;

__attribute__((__section__(".user_data"))) const char data[1];

int main()
{
    app_func_t app;
    uint32_t jump_addr, vt_offset, sp_addr;
    config_t *config = (config_t *)data;

    uart_init();
    print(VERSION);

    print("Start application: ");
    if (!config->active_image)
    {
        print("0\r\n");
        vt_offset = APP1_ADDRESS_OFFSET;
        jump_addr = *(__IO uint32_t *)(APP1_ADDRESS + 4);
        sp_addr = *(__IO uint32_t *)APP1_ADDRESS;
    }
    else
    {
        print("1\r\n");
        vt_offset = APP2_ADDRESS_OFFSET;
        jump_addr = *(__IO uint32_t *)(APP2_ADDRESS + 4);
        sp_addr = *(__IO uint32_t *)APP2_ADDRESS;
    }

    /* Relocate vector table */
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, vt_offset);
    /* Initialize application's Stack Pointer */
    __set_MSP(sp_addr);
    /* Start application */
    app = (app_func_t)jump_addr;
    app();

    return 0;
}

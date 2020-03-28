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
 * | 16K       |
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

#define APP_ADDRESS_OFFSET 0x4000
#define APP_ADDRESS (FLASH_BASE + APP_ADDRESS_OFFSET)

typedef void (*app_func_t)(void);

int main()
{
    app_func_t app;
    uint32_t jump_addr;

    uart_init();
    print(VERSION);

    /* Relocate vector table */
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, APP_ADDRESS_OFFSET);
    /* Set application address */
    jump_addr = *(__IO uint32_t *)(APP_ADDRESS + 4);
    app = (app_func_t)jump_addr;
    /* Initialize application's Stack Pointer */
    __set_MSP(*(__IO uint32_t *)APP_ADDRESS);
    /* Start application */
    app();

    return 0;
}

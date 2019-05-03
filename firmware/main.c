/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "nand_programmer.h"
#include "usb.h"
#include "cdc.h"
#include "led.h"
#include "uart.h"
#include "jtag.h"
#include "version.h"
#include <stdio.h>

int main()
{
    uart_init();
    printf("\r\nNAND programmer ver: %d.%d.%d\r\n", SW_VERSION_MAJOR,
        SW_VERSION_MINOR, SW_VERSION_BUILD);

    printf("JTAG init...");
    jtag_init();
    printf("done.\r\n");

    printf("LED init...");
    led_init();
    printf("done.\r\n");

    printf("USB init...");
    usb_init();
    printf("done.\r\n");

    printf("CDC init...");
    cdc_init();
    printf("done.\r\n");

    while (1)
        np_handler();

    return 0;
}

/*  Copyright (C) 2020 NANDO authors
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
#include "clock.h"
#include <stdio.h>

int main()
{
    uart_init();
    printf("\r\nNAND programmer ver: %d.%d.%d\r\n", SW_VERSION_MAJOR,
        SW_VERSION_MINOR, SW_VERSION_BUILD);

    if (!is_external_clock_avail())
        printf("External clock not detected. Fallback to internal clock.\r\n");

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

    printf("Programmer init...");
    np_init();
    printf("done.\r\n");

    while (1)
        np_handler();

    return 0;
}

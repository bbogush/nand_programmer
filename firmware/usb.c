/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "hw_config.h"
#include <usb_lib.h>
#include <usb_pwr.h>

void usb_init()
{
    Set_System();
    Set_USBClock();
    USB_Interrupts_Config();
    USB_Init();
    while (!USB_IsDeviceConfigured());
}

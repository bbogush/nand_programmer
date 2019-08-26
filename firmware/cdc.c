/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "log.h"
#include "nand_programmer.h"
#include <usb_cdc/hw_config.h>

#define SEND_TIMEOUT 0x1000000

static int cdc_send(uint8_t *data, uint32_t len)
{
    uint32_t timeout = SEND_TIMEOUT;

    if (!CDC_IsPacketSent())
    {
        DEBUG_PRINT("Wait for previous CDC TX\r\n");
        while (!CDC_IsPacketSent() && --timeout);
        if (!timeout)
        {
            ERROR_PRINT("Failed to send data, CDC is busy\r\n");
            return -1;
        }
    }

    if (!CDC_Send_DATA(data, len))
    {
        ERROR_PRINT("Failed to send data\r\n");
        return -1;

    }

    return 0;
}

static int cdc_send_ready()
{
    return CDC_IsPacketSent();
}

static uint32_t cdc_peek(uint8_t **data)
{
    return USB_Data_Peek(data);
}

static void cdc_consume()
{
    uint8_t *data;

    USB_Data_Get(&data);
    USB_DataRx_Sched();
}

static np_comm_cb_t cdc_comm_cb = 
{
    .send = cdc_send,
    .send_ready = cdc_send_ready,
    .peek = cdc_peek,
    .consume = cdc_consume,
};

void cdc_init()
{
    np_comm_register(&cdc_comm_cb);

    /* Enable receive of data */
    CDC_Receive_DATA();
}

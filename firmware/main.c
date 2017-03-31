/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

/* NAND */
#include "fsmc_nand.h"
#include "chip_db.h"
/* SPL */
#include <stm32f10x.h>
/* USB */
#include <usb_lib.h>
#include <usb_pwr.h>
#include "hw_config.h"
/* STD */
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#define NAND_PAGE_NUM      2
#define NAND_BUFFER_SIZE   (NAND_PAGE_NUM * NAND_PAGE_SIZE)

#define USB_BUF_SIZE 60

enum
{
    CMD_NAND_READ_ID = 0x00,
    CMD_NAND_ERASE   = 0x01,
    CMD_NAND_READ    = 0x02,
    CMD_NAND_WRITE_S = 0x03,
    CMD_NAND_WRITE_D = 0x04,
    CMD_NAND_WRITE_E = 0x05,
    CMD_NAND_SELECT  = 0x06,
};

typedef struct __attribute__((__packed__))
{
    uint8_t code;
} cmd_t;

typedef struct __attribute__((__packed__))
{
    cmd_t cmd;
    uint32_t addr;
} write_start_cmd_t;

typedef struct __attribute__((__packed__))
{
    cmd_t cmd;
    uint8_t len;
    uint8_t data[];
} write_data_cmd_t;

typedef struct __attribute__((__packed__))
{
    cmd_t cmd;
} write_end_cmd_t;

typedef struct __attribute__((__packed__))
{
    cmd_t cmd;
    uint32_t addr;
    uint32_t len;
} read_cmd_t;

typedef struct __attribute__((__packed__))
{
    cmd_t cmd;
    uint32_t chip_num;
} select_cmd_t;

enum
{
    RESP_DATA   = 0x00,
    RESP_STATUS = 0x01,
};

typedef struct __attribute__((__packed__))
{
    uint8_t code;
    uint8_t info;
    uint8_t data[];
} resp_t;

enum
{
    STATUS_OK    = 0x00,
    STATUS_ERROR = 0x01,
};

typedef struct __attribute__((__packed__))
{
    resp_t header;
    nand_id_t nand_id;
} resp_id_t;

typedef struct
{
    nand_addr_t addr;
    int is_valid;
} prog_addr_t;

typedef struct
{
    uint8_t buf[NAND_PAGE_SIZE];
    uint32_t offset;
} page_t;

nand_addr_t nand_write_read_addr = { 0x00, 0x00, 0x00 };
uint8_t nand_write_buf[NAND_BUFFER_SIZE], nand_read_buf[NAND_BUFFER_SIZE];

extern __IO uint8_t Receive_Buffer[USB_BUF_SIZE];
extern __IO uint32_t Receive_length;
uint32_t packet_sent = 1;
uint32_t packet_receive = 1;
uint8_t usb_send_buf[USB_BUF_SIZE];
static uint32_t selected_chip;

static void jtag_init()
{
    /* Enable JTAG in low power mode */
    DBGMCU_Config(DBGMCU_SLEEP | DBGMCU_STANDBY | DBGMCU_STOP, ENABLE);
}

static void usb_init()
{
    Set_System();
    Set_USBClock();
    USB_Interrupts_Config();
    USB_Init();
}

static int make_status(uint8_t *buf, size_t buf_size, int is_ok)
{
    resp_t status = { RESP_STATUS,  is_ok ? STATUS_OK : STATUS_ERROR };
    size_t len = sizeof(status);

    if (len > buf_size)
        return -1;

    memcpy(buf, &status, len);

    return len;
}

static int prog_nand_read_id(uint8_t *buf, size_t buf_size)
{
    resp_id_t resp;
    size_t resp_len = sizeof(resp);

    if (buf_size < resp_len)
        goto Error;

    resp.header.code = RESP_DATA;
    resp.header.info = resp_len - sizeof(resp.header);
    nand_read_id(&resp.nand_id);

    memcpy(buf, &resp, resp_len);

    return resp_len;

Error:
    return make_status(usb_send_buf, buf_size, 0);
}

static int prog_nand_erase(uint8_t *buf, size_t buf_size)
{
    uint32_t status;

    /* Erase the NAND first Block */
    status = nand_erase_block(nand_write_read_addr);

    return make_status(buf, buf_size, status == NAND_READY);
}

static int nand_write_start(uint8_t *rx_buf, prog_addr_t *prog_addr,
    page_t *page)
{
    write_start_cmd_t *write_start_cmd = (write_start_cmd_t *)rx_buf;

    if (nand_raw_addr_to_nand_addr(write_start_cmd->addr, &prog_addr->addr)
        != NAND_VALID_ADDRESS)
    {
        return -1;
    }
    prog_addr->is_valid = 1;

    page->offset = write_start_cmd->addr % sizeof(page->buf);
    memset(page->buf, 0, sizeof(page->buf));

    return 0;
}

static int nand_write_data(uint8_t *rx_buf, size_t rx_buf_size,
    prog_addr_t *prog_addr, page_t *page)
{
    uint32_t status, write_len, bytes_left;
    uint32_t page_size = sizeof(page->buf);    
    write_data_cmd_t *write_data_cmd = (write_data_cmd_t *)rx_buf;

    if (write_data_cmd->len + offsetof(write_data_cmd_t, data) > rx_buf_size)
        return -1;

    if (!prog_addr->is_valid)
        return -1;

    if (page->offset + write_data_cmd->len > page_size)
        write_len = page_size - page->offset;
    else
        write_len = write_data_cmd->len;

    memcpy(page->buf + page->offset, write_data_cmd->data, write_len);
    page->offset += write_len;

    if (page->offset == page_size)
    {
        status = nand_write_small_page(page->buf, prog_addr->addr, 1);
        if (!(status & NAND_READY))
            return -1;

        status = nand_addr_inc(&prog_addr->addr);
        if (!(status & NAND_VALID_ADDRESS))
            prog_addr->is_valid = 0;

        page->offset = 0;
        memset(page->buf, 0, page_size);
    }

    bytes_left = write_data_cmd->len - write_len;
    if (bytes_left)
    {
        memcpy(page->buf, write_data_cmd->data + write_len, bytes_left);
        page->offset += bytes_left;
    }

    return 0;
}

static int nand_write_end(prog_addr_t *prog_addr, page_t *page)
{
    uint32_t status;

    if (!prog_addr->is_valid)
        return 0;

    prog_addr->is_valid = 0;

    if (!page->offset)
        return 0;

    status = nand_write_small_page(page->buf, prog_addr->addr, 1);
    if (!(status & NAND_READY))
       return -1;

    return 0;
}

static int prog_nand_write(uint8_t *rx_buf, size_t rx_buf_size, uint8_t *tx_buf,
    size_t tx_buf_size)
{
    static prog_addr_t prog_addr;
    static page_t page;
    cmd_t *cmd = (cmd_t *)rx_buf;
    int ret = 0;

    switch (cmd->code)
    {
    case CMD_NAND_WRITE_S:
        ret = nand_write_start(rx_buf, &prog_addr, &page);
        break;
    case CMD_NAND_WRITE_D:
        ret = nand_write_data(rx_buf, rx_buf_size, &prog_addr, &page);
        if (!ret)
            return 0;
        break;
    case CMD_NAND_WRITE_E:
        ret = nand_write_end(&prog_addr, &page);
        break;
    default:
        ret = -1;
        break;
    }

    return make_status(tx_buf, tx_buf_size, !ret);
}

static int prog_nand_read(uint8_t *rx_buf, size_t rx_buf_size, uint8_t *tx_buf,
    size_t tx_buf_size)
{
    prog_addr_t prog_addr;
    static page_t page;
    uint32_t status, write_len;
    uint32_t page_size = sizeof(page.buf);
    uint32_t resp_header_size = offsetof(resp_t, data);
    uint32_t tx_data_len = tx_buf_size - resp_header_size;
    read_cmd_t *read_cmd = (read_cmd_t *)rx_buf;
    resp_t *resp = (resp_t *)tx_buf;

    if (nand_raw_addr_to_nand_addr(read_cmd->addr, &prog_addr.addr)
        != NAND_VALID_ADDRESS)
    {
        goto Error;
    }

    page.offset = read_cmd->addr % page_size;

    resp->code = RESP_DATA;

    while (read_cmd->len)
    {
        status = nand_read_small_page(page.buf, prog_addr.addr, 1);
        if (!(status & NAND_READY))
            goto Error;

        while (page.offset < page_size && read_cmd->len)
        {
            if (page_size - page.offset >= tx_data_len)
                write_len = tx_data_len;
            else
                write_len = page_size - page.offset;

            if (write_len > read_cmd->len)
                write_len = read_cmd->len;
 
            memcpy(resp->data, page.buf + page.offset, write_len);

            while (!packet_sent);

            resp->info = write_len;
            CDC_Send_DATA(tx_buf, resp_header_size + write_len);

            page.offset += write_len;
            if (page.offset == page_size)
                page.offset = 0;
            read_cmd->len -= write_len;
        }

        if (read_cmd->len)
        {
            status = nand_addr_inc(&prog_addr.addr);
            if (!(status & NAND_VALID_ADDRESS))
                goto Error;
        }
    }

    return 0;

Error:
    return make_status(tx_buf, tx_buf_size, 0);
}

static int prog_nand_select(uint8_t *rx_buf, size_t rx_buf_size,
    uint8_t *tx_buf, size_t tx_buf_size)
{
    select_cmd_t *select_cmd = (select_cmd_t *)rx_buf;
    int ret = 0;

    if (select_cmd->chip_num >= CHIP_ID_LAST)
        ret = -1;
    else
        selected_chip = select_cmd->chip_num;

    return make_status(tx_buf, tx_buf_size, !ret);
}

static int cmd_handler(uint8_t *rx_buf, size_t rx_buf_size, uint8_t *tx_buf,
    size_t tx_buf_size)
{
    cmd_t *cmd = (cmd_t *)rx_buf;
    int ret = -1;

    switch (cmd->code)
    {
    case CMD_NAND_READ_ID:
        ret = prog_nand_read_id(tx_buf, tx_buf_size);
        break;
    case CMD_NAND_ERASE:
        ret = prog_nand_erase(tx_buf, tx_buf_size);
        break;
    case CMD_NAND_READ:
        ret = prog_nand_read(rx_buf, rx_buf_size, tx_buf, tx_buf_size);
        break;
    case CMD_NAND_WRITE_S:
    case CMD_NAND_WRITE_D:
    case CMD_NAND_WRITE_E:
        ret = prog_nand_write(rx_buf, rx_buf_size, tx_buf, tx_buf_size);
        break;
    case CMD_NAND_SELECT:
        ret = prog_nand_select(rx_buf, rx_buf_size, tx_buf, tx_buf_size);
        break;
    default:
        break;
    }

    return ret;
}

static void usb_handler()
{
    int len;

    if (bDeviceState != CONFIGURED)
        return;

    CDC_Receive_DATA();
    if (!Receive_length)
        return;

    len = cmd_handler((uint8_t *)Receive_Buffer, sizeof(Receive_Buffer),
        usb_send_buf, sizeof(usb_send_buf));
    if (len <= 0)
        goto Exit;

    if (packet_sent)
        CDC_Send_DATA(usb_send_buf, len);

Exit:
    Receive_length = 0;
}

int main()
{
    jtag_init();

    usb_init();

    nand_init(CHIP_ID_K9F2G08U0C);

    while (1)
        usb_handler();

    return 0;
}

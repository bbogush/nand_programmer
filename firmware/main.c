/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

/* NAND */
#include "fsmc_nand.h"
#include "chip_db.h"
#include "nand_programmer.h"
/* SPL */
#include <stm32f10x.h>
/* USB */
#include <usb_lib.h>
#include <usb_pwr.h>
#include "hw_config.h"
#include "cdc.h"
/* LED */
#include "led.h"
/* UART */
#include "uart.h"
/* JTAG */
#include "jtag.h"
/* Version */
#include "version.h"
/* Utils */
#include "log.h"
/* STD */
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

#define USB_BUF_SIZE 64
#define MAX_PAGE_SIZE 0x0800
#define WRITE_ACK_BYTES 1984
#define NAND_TIMEOUT 0x1000000

#define NAND_GOOD_BLOCK_MARK 0xFF

enum
{
    CMD_NAND_READ_ID = 0x00,
    CMD_NAND_ERASE   = 0x01,
    CMD_NAND_READ    = 0x02,
    CMD_NAND_WRITE_S = 0x03,
    CMD_NAND_WRITE_D = 0x04,
    CMD_NAND_WRITE_E = 0x05,
    CMD_NAND_SELECT  = 0x06,
    CMD_NAND_READ_BB = 0x07,
};

enum
{
    ERR_ADDR_EXCEEDED  = 0x00,
    ERR_ADDR_INVALID   = 0x01,
    ERR_NAND_WR        = 0x02,
    ERR_NAND_RD        = 0x03,
    ERR_NAND_ERASE     = 0x04,
    ERR_CHIP_NOT_SEL   = 0x05,
    ERR_CHIP_NOT_FOUND = 0x06,
    ERR_CMD_DATA_SIZE  = 0x07,
    ERR_BUF_OVERFLOW   = 0x08,
};

typedef struct __attribute__((__packed__))
{
    uint8_t code;
} cmd_t;

typedef struct __attribute__((__packed__))
{
    cmd_t cmd;
    uint32_t addr;
    uint32_t len;
} erase_cmd_t;

typedef struct __attribute__((__packed__))
{
    cmd_t cmd;
    uint32_t addr;
    uint32_t len;
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
    STATUS_OK        = 0x00,
    STATUS_ERROR     = 0x01,
    STATUS_BAD_BLOCK = 0x02,
    STATUS_WRITE_ACK = 0x03,
};

typedef struct __attribute__((__packed__))
{
    resp_t header;
    nand_id_t nand_id;
} resp_id_t;

typedef struct __attribute__((__packed__))
{
    resp_t header;
    uint32_t addr;
} resp_bad_block_t;

typedef struct __attribute__((__packed__))
{
    resp_t header;
    uint32_t bytes_ack;
} resp_write_ack_t;

typedef struct __attribute__((__packed__))
{
    resp_t header;
    uint8_t err_code;
} resp_err_t;

typedef struct
{
    uint32_t addr;
    int is_valid;
} prog_addr_t;

typedef struct
{
    uint8_t buf[MAX_PAGE_SIZE];
    uint32_t page;
    uint32_t offset;
} page_t;

typedef struct
{
    uint8_t *rx_buf;
    uint8_t rx_buf_size;
    uint8_t *tx_buf;
    uint8_t tx_buf_size;
} usb_t;

typedef struct
{
    usb_t usb;
    uint32_t addr;
    uint32_t len;
    int addr_is_valid;
    page_t page;
    uint32_t bytes_written;
    uint32_t bytes_ack;
    int nand_wr_in_progress;
    uint32_t nand_timeout;
    chip_info_t *chip_info;
} prog_t;

static np_comm_cb_t *np_comm_cb;

uint8_t usb_send_buf[USB_BUF_SIZE];

static void usb_init(usb_t *usb)
{
    Set_System();
    Set_USBClock();
    USB_Interrupts_Config();
    USB_Init();
    while (!USB_IsDeviceConfigured());    

    usb->tx_buf = (uint8_t *)usb_send_buf;
    usb->tx_buf_size = USB_BUF_SIZE;
    usb->rx_buf_size = USB_BUF_SIZE;
}

static int make_ok_status(usb_t *usb)
{
    resp_t status = { RESP_STATUS, STATUS_OK };
    size_t len = sizeof(status);

    if (len > usb->tx_buf_size)
    {
        ERROR_PRINT("Status size %d is more then TX buffer size %d\r\n", len,
            usb->tx_buf_size);
        return -1;
    }

    memcpy(usb->tx_buf, &status, len);

    return len;
}

static int make_error_status(usb_t *usb, uint8_t err_code)
{
    resp_t status = { RESP_STATUS, STATUS_ERROR };
    resp_err_t err_status = { status, err_code };
    size_t len = sizeof(err_status);

    if (len > usb->tx_buf_size)
    {
        ERROR_PRINT("Status size %d is more then TX buffer size %d\r\n", len,
            usb->tx_buf_size);
        return -1;
    }

    memcpy(usb->tx_buf, &err_status, len);

    return len;
}

static int send_bad_block_info(uint32_t addr)
{
    resp_t resp_header = { RESP_STATUS, STATUS_BAD_BLOCK };
    resp_bad_block_t bad_block = { resp_header, addr };

    if (np_comm_cb->send((uint8_t *)&bad_block, sizeof(bad_block)))
        return -1;

    return 0;
}

static int cmd_nand_read_id(prog_t *prog)
{
    resp_id_t resp;
    size_t resp_len = sizeof(resp);

    DEBUG_PRINT("Read ID command\r\n");

    if (prog->usb.tx_buf_size < resp_len)
    {
        ERROR_PRINT("Response size is more then TX buffer size\r\n");
        return make_error_status(&prog->usb, ERR_BUF_OVERFLOW);
    }

    resp.header.code = RESP_DATA;
    resp.header.info = resp_len - sizeof(resp.header);
    nand_read_id(&resp.nand_id);

    memcpy(prog->usb.tx_buf, &resp, resp_len);

    return resp_len;
}

static int nand_erase(uint32_t page, uint32_t addr)
{
    uint32_t status;
    
    DEBUG_PRINT("NAND erase at 0x%lx\r\n", addr);

    status = nand_erase_block(page);
    switch (status)
    {
    case NAND_READY:
        break;
    case NAND_ERROR:
        if (send_bad_block_info(addr))
            return -1;
        break;
    case NAND_TIMEOUT_ERROR:
        ERROR_PRINT("NAND erase timeout at 0x%lx\r\n", addr);
        break;
    default:
        ERROR_PRINT("Unknown NAND status\r\n");
        return -1;
    }

    return 0;
}

static int cmd_nand_erase(prog_t *prog)
{
    uint32_t addr, page, pages_in_block;
    erase_cmd_t *erase_cmd = (erase_cmd_t *)prog->usb.rx_buf;

    DEBUG_PRINT("Erase at 0x%lx %lx bytes command\r\n", erase_cmd->addr,
        erase_cmd->len);

    addr = erase_cmd->addr & ~(prog->chip_info->block_size - 1);
    erase_cmd->len += erase_cmd->addr - addr;

    page = addr / prog->chip_info->page_size;
    pages_in_block = prog->chip_info->block_size / prog->chip_info->page_size;

    while (erase_cmd->len)
    {
        if (addr >= prog->chip_info->size)
        {
            ERROR_PRINT("Erase address 0x%lx is more then chip size 0x%lx\r\n",
                addr, prog->chip_info->size);
            return make_error_status(&prog->usb, ERR_ADDR_EXCEEDED);
        }

        if (nand_erase(page, addr))
            return make_error_status(&prog->usb, ERR_NAND_ERASE);

        if (erase_cmd->len >= prog->chip_info->block_size)
            erase_cmd->len -= prog->chip_info->block_size;
        else
            erase_cmd->len = 0;
        addr += prog->chip_info->block_size;
        page += pages_in_block;
    }

    return make_ok_status(&prog->usb);
}

static int send_write_ack(uint32_t bytes_ack)
{
    resp_t resp_header = { RESP_STATUS, STATUS_WRITE_ACK };
    resp_write_ack_t write_ack = { resp_header, bytes_ack };

    if (np_comm_cb->send((uint8_t *)&write_ack, sizeof(write_ack)))
        return -1;

    return 0;
}

static int cmd_nand_write_start(prog_t *prog)
{
    write_start_cmd_t *write_start_cmd = (write_start_cmd_t *)prog->usb.rx_buf;

    DEBUG_PRINT("Write at 0x%lx command\r\n", write_start_cmd->addr);

    if (write_start_cmd->addr >= prog->chip_info->size)
    {
        ERROR_PRINT("Write address 0x%lx is more then chip size 0x%lx\r\n",
            write_start_cmd->addr, prog->chip_info->size);
        return make_error_status(&prog->usb, ERR_ADDR_EXCEEDED);
    }

    prog->addr = write_start_cmd->addr;
    prog->len = write_start_cmd->len;
    prog->addr_is_valid = 1;

    prog->page.page = write_start_cmd->addr / prog->chip_info->page_size;
    prog->page.offset = write_start_cmd->addr % prog->chip_info->page_size;
    memset(prog->page.buf, 0, sizeof(prog->page.buf));

    prog->bytes_written = 0;
    prog->bytes_ack = 0;

    return make_ok_status(&prog->usb);
}

static int nand_handle_status(prog_t *prog)
{
    switch (nand_read_status())
    {
    case NAND_ERROR:
        if (send_bad_block_info(prog->addr))
            return -1;
    case NAND_READY:
        prog->nand_wr_in_progress = 0;
        prog->nand_timeout = 0;
        break;
    case NAND_BUSY:
        if (++prog->nand_timeout == NAND_TIMEOUT)
        {
            ERROR_PRINT("NAND write timeout at 0x%lx\r\n", prog->addr);
            prog->nand_wr_in_progress = 0;
            prog->nand_timeout = 0;
            return -1;
        }
        break;
    default:
        ERROR_PRINT("Unknown NAND status\r\n");
        prog->nand_wr_in_progress = 0;
        prog->nand_timeout = 0;
        return -1;
    }

    return 0;
}

static int nand_write(prog_t *prog, chip_info_t *chip_info)
{   
    if (prog->nand_wr_in_progress)
    {
        DEBUG_PRINT("Wait for previous NAND write\r\n");
        do
        {
            if (nand_handle_status(prog))
                return -1;
        }
        while (prog->nand_wr_in_progress);
    }

    DEBUG_PRINT("NAND write at 0x%lx %lu bytes\r\n", prog->addr,
        chip_info->page_size);

    nand_write_page_async(prog->page.buf, prog->page.page,
        chip_info->page_size);

    prog->nand_wr_in_progress = 1;

    return 0;
}

static int send_status(prog_t *prog, int len)
{
    if (np_comm_cb)
        np_comm_cb->send(prog->usb.tx_buf, len);

    return 0;
}

static int cmd_nand_write_data(prog_t *prog)
{
    uint32_t write_len, bytes_left;
    write_data_cmd_t *write_data_cmd = (write_data_cmd_t *)prog->usb.rx_buf;

    if (write_data_cmd->len + offsetof(write_data_cmd_t, data) >
        prog->usb.rx_buf_size)
    {
        ERROR_PRINT("Data size is wrong %d\r\n", write_data_cmd->len);
        return make_error_status(&prog->usb, ERR_CMD_DATA_SIZE);
    }

    if (!prog->addr_is_valid)
    {
        ERROR_PRINT("Write address is not set\r\n");
        return make_error_status(&prog->usb, ERR_ADDR_INVALID);
    }

    if (prog->page.offset + write_data_cmd->len > prog->chip_info->page_size)
        write_len = prog->chip_info->page_size - prog->page.offset;
    else
        write_len = write_data_cmd->len;

    memcpy(prog->page.buf + prog->page.offset, write_data_cmd->data, write_len);
    prog->page.offset += write_len;

    if (prog->page.offset == prog->chip_info->page_size)
    {
        if (nand_write(prog, prog->chip_info))
            return make_error_status(&prog->usb, ERR_NAND_WR);

        prog->addr += prog->chip_info->page_size;
        if (prog->addr >= prog->chip_info->size)
            prog->addr_is_valid = 0;

        prog->page.page++;
        prog->page.offset = 0;
        memset(prog->page.buf, 0, prog->chip_info->page_size);
    }

    bytes_left = write_data_cmd->len - write_len;
    if (bytes_left)
    {
        memcpy(prog->page.buf, write_data_cmd->data + write_len, bytes_left);
        prog->page.offset += bytes_left;
    }

    prog->bytes_written += write_data_cmd->len;
    if (prog->bytes_written - prog->bytes_ack >= prog->chip_info->page_size ||
        prog->bytes_written == prog->len)
    {
        if (send_write_ack(prog->bytes_written))
            return -1;
        prog->bytes_ack = prog->bytes_written;
    }

    return 0;
}

static int cmd_nand_write_end(prog_t *prog)
{
    if (!prog->page.offset)
        goto Exit;

    if (!prog->addr_is_valid)
    {
        ERROR_PRINT("Write address is not set\r\n");
        return make_error_status(&prog->usb, ERR_ADDR_INVALID);
    }

    prog->addr_is_valid = 0;

    if (nand_write(prog, prog->chip_info))
        return make_error_status(&prog->usb, ERR_NAND_WR);

Exit:
    return make_ok_status(&prog->usb);
}

static int cmd_nand_write(prog_t *prog)
{
    cmd_t *cmd = (cmd_t *)prog->usb.rx_buf;
    int ret = 0;

    switch (cmd->code)
    {
    case CMD_NAND_WRITE_S:
        ret = cmd_nand_write_start(prog);
        break;
    case CMD_NAND_WRITE_D:
        ret = cmd_nand_write_data(prog);
        break;
    case CMD_NAND_WRITE_E:
        ret = cmd_nand_write_end(prog);
        break;
    default:
        break;
    }

    return ret;
}

static int nand_read(uint32_t addr, page_t *page, chip_info_t *chip_info)
{
    uint32_t status;

    DEBUG_PRINT("NAND read at 0x%lx\r\n", addr);    
    
    status = nand_read_page(page->buf, page->page, chip_info->page_size);
    switch (status)
    {
    case NAND_READY:
        break;
    case NAND_ERROR:
        if (send_bad_block_info(addr))
            return -1;
        break;
    case NAND_TIMEOUT_ERROR:
        ERROR_PRINT("NAND write timeout at 0x%lx\r\n", addr);
        break;
    default:
        ERROR_PRINT("Unknown NAND status\r\n");
        return -1;
    }

    return 0;
}

static int cmd_nand_read(prog_t *prog)
{
    uint32_t addr;
    static page_t page;
    uint32_t write_len;
    uint32_t resp_header_size = offsetof(resp_t, data);
    uint32_t tx_data_len = prog->usb.tx_buf_size - resp_header_size;
    read_cmd_t *read_cmd = (read_cmd_t *)prog->usb.rx_buf;
    resp_t *resp = (resp_t *)prog->usb.tx_buf;

    DEBUG_PRINT("Read at 0x%lx %lx bytes command\r\n", read_cmd->addr,
        read_cmd->len);

    if (read_cmd->addr >= prog->chip_info->size)
    {
        ERROR_PRINT("Read address 0x%lx is more then chip size 0x%lx\r\n",
            read_cmd->addr, prog->chip_info->size);
        return make_error_status(&prog->usb, ERR_ADDR_EXCEEDED);
    }

    addr = read_cmd->addr;
    page.page = addr / prog->chip_info->page_size;
    page.offset = addr % prog->chip_info->page_size;

    resp->code = RESP_DATA;

    while (read_cmd->len)
    {
        if (nand_read(addr, &page, prog->chip_info))
            return make_error_status(&prog->usb, ERR_NAND_RD);

        while (page.offset < prog->chip_info->page_size && read_cmd->len)
        {
            if (prog->chip_info->page_size - page.offset >= tx_data_len)
                write_len = tx_data_len;
            else
                write_len = prog->chip_info->page_size - page.offset;

            if (write_len > read_cmd->len)
                write_len = read_cmd->len;
 
            memcpy(resp->data, page.buf + page.offset, write_len);

            while (!np_comm_cb->send_ready());

            resp->info = write_len;
            if (np_comm_cb->send(prog->usb.tx_buf,
                resp_header_size + write_len))
            {
                return -1;
            }

            page.offset += write_len;
            read_cmd->len -= write_len;
        }

        if (read_cmd->len)
        {
            addr += prog->chip_info->page_size;
            if (addr >= prog->chip_info->size)
            {
                ERROR_PRINT("Read address 0x%lx is more then chip size 0x%lx",
                    addr, prog->chip_info->page_size);
                return make_error_status(&prog->usb, ERR_ADDR_EXCEEDED);
            }
            page.page++;
            page.offset = 0;
        }
    }

    return 0;
}

static int cmd_nand_select(prog_t *prog)
{
    select_cmd_t *select_cmd = (select_cmd_t *)prog->usb.rx_buf;

    DEBUG_PRINT("Chip select ID %lu command\r\n", select_cmd->chip_num);

    if (!chip_select(select_cmd->chip_num))
    {
        nand_init();
        prog->chip_info = chip_info_selected_get();
    }
    else
    {
        prog->chip_info = NULL;

        ERROR_PRINT("Chip ID %lu not found\r\n", select_cmd->chip_num);
        return make_error_status(&prog->usb, ERR_CHIP_NOT_FOUND);
    }

    return make_ok_status(&prog->usb);
}

static int read_bad_block_info_from_page(prog_t *prog, uint32_t block,
    uint32_t page, chip_info_t *chip_info, bool *is_bad)
{
    uint8_t bad_block_data;
    uint32_t status, addr = block * chip_info->block_size;

    status = nand_read_data(&bad_block_data, page, chip_info->page_size,
        sizeof(bad_block_data));
    switch (status)
    {
    case NAND_READY:
        break;
    case NAND_ERROR:
        ERROR_PRINT("NAND read bad block info error at 0x%lx\r\n", addr);
        return make_error_status(&prog->usb, ERR_NAND_RD);
    case NAND_TIMEOUT_ERROR:
        ERROR_PRINT("NAND read timeout at 0x%lx\r\n", addr);
        return make_error_status(&prog->usb, ERR_NAND_RD);
    default:
        ERROR_PRINT("Unknown NAND status\r\n");
        return make_error_status(&prog->usb, ERR_NAND_RD);
    }

    if (bad_block_data != NAND_GOOD_BLOCK_MARK)
    {
        *is_bad = true;
        if (send_bad_block_info(addr))
            return -1;
    }
    else
        *is_bad = false;

    return 0;
}

static int cmd_read_bad_blocks(prog_t *prog)
{
    bool is_bad;
    uint32_t block, block_num, page_num, page;

    block_num = prog->chip_info->size / prog->chip_info->block_size;
    page_num = prog->chip_info->block_size / prog->chip_info->page_size;

    /* Bad block - not 0xFF value in the first or second page in the block at
     * zero offset in the page spare area
     */
    for (block = 0; block < block_num; block++)
    {
        page = block * page_num;
        if (read_bad_block_info_from_page(prog, block, page, prog->chip_info,
            &is_bad))
        {
            return -1;
        }

        if (!is_bad && read_bad_block_info_from_page(prog, block, page + 1,
            prog->chip_info, &is_bad))
        {
            return -1;
        }
    }

    return make_ok_status(&prog->usb);
}

static int usb_cmd_handler(prog_t *prog)
{
    cmd_t *cmd = (cmd_t *)prog->usb.rx_buf;
    int ret = -1;

    if (!prog->chip_info && cmd->code != CMD_NAND_SELECT)
    {
        ERROR_PRINT("Chip is not selected\r\n");
        return make_error_status(&prog->usb, ERR_CHIP_NOT_SEL);
    }

    switch (cmd->code)
    {
    case CMD_NAND_READ_ID:
        led_rd_set(true);
        ret = cmd_nand_read_id(prog);
        led_rd_set(false);
        break;
    case CMD_NAND_ERASE:
        led_wr_set(true);
        ret = cmd_nand_erase(prog);
        led_wr_set(false);
        break;
    case CMD_NAND_READ:
        led_rd_set(true);
        ret = cmd_nand_read(prog);
        led_rd_set(false);
        break;
    case CMD_NAND_WRITE_S:
        led_wr_set(true);
        ret = cmd_nand_write(prog);
        break;
    case CMD_NAND_WRITE_D:
        ret = cmd_nand_write(prog);
        break;
    case CMD_NAND_WRITE_E:
        ret = cmd_nand_write(prog);
        led_wr_set(false);
        break;
    case CMD_NAND_SELECT:
        ret = cmd_nand_select(prog);
        break;
    case CMD_NAND_READ_BB:
        led_rd_set(true);
        ret = cmd_read_bad_blocks(prog);
        led_rd_set(false);
        break;
    default:
        break;
    }

    return ret;
}

static void cmd_handler(prog_t *prog)
{
    int len;

    do
    {
        np_comm_cb->peek(&prog->usb.rx_buf);

        if (!prog->usb.rx_buf)
            break;

        len = usb_cmd_handler(prog);

        np_comm_cb->consume();

        if (len <= 0)
            continue;

        send_status(prog, len);
    } while (true);
}

static void nand_handler(prog_t *prog)
{
    if (prog->nand_wr_in_progress)
    {
        if (nand_handle_status(prog))
            send_status(prog, make_error_status(&prog->usb, ERR_NAND_WR));
    }
}

int np_comm_register(np_comm_cb_t *cb)
{
    np_comm_cb = cb;

    return 0;
}

void np_comm_unregister(np_comm_cb_t *cb)
{
    if (np_comm_cb == cb)
        np_comm_cb = NULL;
}

int main()
{
    static prog_t prog;

    uart_init();
    printf("\r\nNAND programmer ver: "SW_VERSION"\r\n");

    printf("JTAG init...");
    jtag_init();
    printf("done.\r\n");

    printf("LED init...");
    led_init();
    printf("done.\r\n");

    printf("USB init...");
    usb_init(&prog.usb);
    printf("done.\r\n");

    printf("CDC init...");
    cdc_init();
    printf("done.\r\n");

    while (1)
    {
        cmd_handler(&prog);
        nand_handler(&prog);
    }

    return 0;
}

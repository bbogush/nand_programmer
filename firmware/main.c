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

#define USB_BUF_SIZE 64
#define MAX_PAGE_SIZE 0x0800
#define WRITE_ACK_BYTES 2048
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
    uint32_t len;
} erase_cmd_t;

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
    int addr_is_valid;
    page_t page;
    uint32_t bytes_written;
    uint32_t bytes_ack;
} prog_t;

extern __IO uint8_t Receive_Buffer[USB_BUF_SIZE];
uint8_t usb_send_buf[USB_BUF_SIZE];

int __io_putchar(int ch)
{
    USART_SendData(USART1, (uint8_t)ch);

    /* Loop until the end of transmission */
    while (!USART_GetFlagStatus(USART1, USART_FLAG_TC));

    return ch;
}

static void uart_init()
{
    USART_InitTypeDef usart;
    GPIO_InitTypeDef usart_gpio;

    /* Enable GPIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, 1);

    /* Enable UART clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, 1); 

    /* Configure USART Tx as alternate function push-pull */
    usart_gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    usart_gpio.GPIO_Pin = GPIO_Pin_9;
    usart_gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &usart_gpio);

    /* Configure USART Rx as input floating */
    usart_gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    usart_gpio.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOA, &usart_gpio);

    /* USART configuration */
    usart.USART_BaudRate = 115200;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &usart);

    /* Enable USART */
    USART_Cmd(USART1, 1);
}

static void jtag_init()
{
    /* Enable JTAG in low power mode */
    DBGMCU_Config(DBGMCU_SLEEP | DBGMCU_STANDBY | DBGMCU_STOP, ENABLE);
}

static void usb_init(usb_t *usb)
{
    Set_System();
    Set_USBClock();
    USB_Interrupts_Config();
    USB_Init();

    usb->rx_buf = (uint8_t *)Receive_Buffer;
    usb->rx_buf_size = sizeof(Receive_Buffer);
    usb->tx_buf = (uint8_t *)usb_send_buf;
    usb->tx_buf_size = sizeof(usb_send_buf);
}

static int make_status(usb_t *usb, int is_ok)
{
    resp_t status = { RESP_STATUS,  is_ok ? STATUS_OK : STATUS_ERROR };
    size_t len = sizeof(status);

    if (len > usb->tx_buf_size)
        return -1;

    memcpy(usb->tx_buf, &status, len);

    return len;
}

static int send_bad_block_info(uint32_t addr)
{
    resp_t resp_header = { RESP_STATUS, STATUS_BAD_BLOCK };
    resp_bad_block_t bad_block = { resp_header, addr };

    if (!CDC_Send_DATA((uint8_t *)&bad_block, sizeof(bad_block)))
        return -1;

    return 0;
}

static int cmd_nand_read_id(prog_t *prog)
{
    resp_id_t resp;
    size_t resp_len = sizeof(resp);

    if (!chip_is_selected())
        goto Error;

    if (prog->usb.tx_buf_size < resp_len)
        goto Error;

    resp.header.code = RESP_DATA;
    resp.header.info = resp_len - sizeof(resp.header);
    nand_read_id(&resp.nand_id);

    memcpy(prog->usb.tx_buf, &resp, resp_len);

    return resp_len;

Error:
    return make_status(&prog->usb, 0);
}

static int cmd_nand_erase(prog_t *prog)
{
    chip_info_t *chip_info;
    uint32_t addr, page, pages_in_block, ret = -1;
    erase_cmd_t *erase_cmd = (erase_cmd_t *)prog->usb.rx_buf;

    if (!chip_is_selected())
        goto Exit;
    chip_info = chip_info_selected_get();

    addr = erase_cmd->addr & ~(chip_info->block_size - 1);
    erase_cmd->len += erase_cmd->addr - addr;

    page = addr / chip_info->page_size;
    pages_in_block = chip_info->block_size / chip_info->page_size;

    while (erase_cmd->len)
    {
        if (addr >= chip_info->size)
            goto Exit;

        if (nand_erase_block(page) != NAND_READY)
        {
            if (nand_read_status() == NAND_ERROR)
            {
                if (send_bad_block_info(addr))
                    goto Exit;
            }
            else
                goto Exit;
        }

        if (erase_cmd->len >= chip_info->block_size)
            erase_cmd->len -= chip_info->block_size;
        else
            erase_cmd->len = 0;
        addr += chip_info->block_size;
        page += pages_in_block;
    }

    ret = 0;
Exit:
    return make_status(&prog->usb, !ret);
}

static int send_write_ack(uint32_t bytes_ack)
{
    resp_t resp_header = { RESP_STATUS, STATUS_WRITE_ACK };
    resp_write_ack_t write_ack = { resp_header, bytes_ack };

    if (!CDC_Send_DATA((uint8_t *)&write_ack, sizeof(write_ack)))
        return -1;

    return 0;
}

static int cmd_nand_write_start(prog_t *prog)
{
    write_start_cmd_t *write_start_cmd = (write_start_cmd_t *)prog->usb.rx_buf;
    chip_info_t *chip_info = chip_info_selected_get();

    if (write_start_cmd->addr >= chip_info->size)
        return -1;

    prog->addr = write_start_cmd->addr;
    prog->addr_is_valid = 1;

    prog->page.page = write_start_cmd->addr / chip_info->page_size;
    prog->page.offset = write_start_cmd->addr % chip_info->page_size;
    memset(prog->page.buf, 0, sizeof(prog->page.buf));

    prog->bytes_written = 0;
    prog->bytes_ack = 0;

    return 0;
}

static int cmd_nand_write_data(prog_t *prog)
{
    uint32_t status, write_len, bytes_left;
    write_data_cmd_t *write_data_cmd = (write_data_cmd_t *)prog->usb.rx_buf;
    chip_info_t *chip_info = chip_info_selected_get();

    if (write_data_cmd->len + offsetof(write_data_cmd_t, data) >
        prog->usb.rx_buf_size)
    {
        return -1;
    }

    if (!prog->addr_is_valid)
        return -1;

    if (prog->page.offset + write_data_cmd->len > chip_info->page_size)
        write_len = chip_info->page_size - prog->page.offset;
    else
        write_len = write_data_cmd->len;

    memcpy(prog->page.buf + prog->page.offset, write_data_cmd->data, write_len);
    prog->page.offset += write_len;

    if (prog->page.offset == chip_info->page_size)
    {
        if ((status = nand_write_page(prog->page.buf, prog->page.page,
            chip_info->page_size)) != NAND_READY)
        {
            if (nand_read_status() == NAND_ERROR)
            {
                if (send_bad_block_info(prog->addr))
                    return -1;
            }
            else
                return -1;
        }

        prog->addr += chip_info->page_size;
        if (prog->addr >= chip_info->size)
            prog->addr_is_valid = 0;

        prog->page.page++;
        prog->page.offset = 0;
        memset(prog->page.buf, 0, chip_info->page_size);
    }

    bytes_left = write_data_cmd->len - write_len;
    if (bytes_left)
    {
        memcpy(prog->page.buf, write_data_cmd->data + write_len, bytes_left);
        prog->page.offset += bytes_left;
    }

    prog->bytes_written += write_data_cmd->len;
    if (prog->bytes_written - prog->bytes_ack >= WRITE_ACK_BYTES)
    {
        if (send_write_ack(prog->bytes_written))
            return -1;
        prog->bytes_ack = prog->bytes_written;
    }

    return 0;
}

static int cmd_nand_write_end(prog_t *prog)
{
    chip_info_t *chip_info = chip_info_selected_get();

    if (!prog->addr_is_valid)
        return 0;

    prog->addr_is_valid = 0;

    if (!prog->page.offset)
        return 0;

    if (nand_write_page(prog->page.buf, prog->page.page, chip_info->page_size)
        != NAND_READY)
    {
       return -1;
    }

    return 0;
}

static int cmd_nand_write(prog_t *prog)
{
    cmd_t *cmd = (cmd_t *)prog->usb.rx_buf;
    int ret = -1;

    if (!chip_is_selected())
        goto Exit;

    switch (cmd->code)
    {
    case CMD_NAND_WRITE_S:
        ret = cmd_nand_write_start(prog);
        break;
    case CMD_NAND_WRITE_D:
        ret = cmd_nand_write_data(prog);
        if (!ret)
            return 0;
        break;
    case CMD_NAND_WRITE_E:
        ret = cmd_nand_write_end(prog);
        break;
    default:
        break;
    }

Exit:
    return make_status(&prog->usb, !ret);
}

static int cmd_nand_read(prog_t *prog)
{
    chip_info_t *chip_info;
    uint32_t addr;
    static page_t page;
    uint32_t status, write_len;
    uint32_t resp_header_size = offsetof(resp_t, data);
    uint32_t tx_data_len = prog->usb.tx_buf_size - resp_header_size;
    read_cmd_t *read_cmd = (read_cmd_t *)prog->usb.rx_buf;
    resp_t *resp = (resp_t *)prog->usb.tx_buf;

    if (!chip_is_selected())
        goto Error;
    chip_info = chip_info_selected_get();

    if (read_cmd->addr >= chip_info->size)
        goto Error;

    addr = read_cmd->addr;
    page.page = addr / chip_info->page_size;
    page.offset = addr % chip_info->page_size;

    resp->code = RESP_DATA;

    while (read_cmd->len)
    {
        status = nand_read_page(page.buf, page.page, chip_info->page_size);
        if (status != NAND_READY)
        {
            if (nand_read_status() == NAND_ERROR)
            {
                if (send_bad_block_info(addr))
                    goto Error;
            }
            else
                goto Error;
        }

        while (page.offset < chip_info->page_size && read_cmd->len)
        {
            if (chip_info->page_size - page.offset >= tx_data_len)
                write_len = tx_data_len;
            else
                write_len = chip_info->page_size - page.offset;

            if (write_len > read_cmd->len)
                write_len = read_cmd->len;
 
            memcpy(resp->data, page.buf + page.offset, write_len);

            while (!CDC_IsPacketSent());

            resp->info = write_len;
            CDC_Send_DATA(prog->usb.tx_buf, resp_header_size + write_len);

            page.offset += write_len;
            read_cmd->len -= write_len;
        }

        if (read_cmd->len)
        {
            addr += chip_info->page_size;
            if (addr >= chip_info->size)
                goto Error;
            page.page++;
            page.offset = 0;
        }
    }

    return 0;

Error:
    return make_status(&prog->usb, 0);
}

static int cmd_nand_select(prog_t *prog)
{
    select_cmd_t *select_cmd = (select_cmd_t *)prog->usb.rx_buf;
    int ret = 0;

    if (!chip_select(select_cmd->chip_num))
        nand_init();
    else
        ret = -1;

    return make_status(&prog->usb, !ret);
}

static int usb_cmd_handler(prog_t *prog)
{
    cmd_t *cmd = (cmd_t *)prog->usb.rx_buf;
    int ret = -1;

    switch (cmd->code)
    {
    case CMD_NAND_READ_ID:
        ret = cmd_nand_read_id(prog);
        break;
    case CMD_NAND_ERASE:
        ret = cmd_nand_erase(prog);
        break;
    case CMD_NAND_READ:
        ret = cmd_nand_read(prog);
        break;
    case CMD_NAND_WRITE_S:
    case CMD_NAND_WRITE_D:
    case CMD_NAND_WRITE_E:
        ret = cmd_nand_write(prog);
        break;
    case CMD_NAND_SELECT:
        ret = cmd_nand_select(prog);
        break;
    default:
        break;
    }

    return ret;
}

static void cmd_handler(prog_t *prog)
{
    int len;

    CDC_Receive_DATA();
    if (!CDC_ReceiveDataLen())
        return;

    len = usb_cmd_handler(prog);
    if (len <= 0)
        goto Exit;

    if (CDC_IsPacketSent())
        CDC_Send_DATA(prog->usb.tx_buf, len);

Exit:
    CDC_ReceiveDataAck();
}

int main()
{
    static prog_t prog;

    uart_init();
    printf("\r\nNAND programmer ver: 1.0\r\n");

    printf("JTAG init...");
    jtag_init();
    printf("send resp.\r\n");
    printf("done.\r\n");

    printf("USB init...");
    usb_init(&prog.usb);
    printf("done.\r\n");

    printf("USB configuring...");
    while (!USB_IsDeviceConfigured());
    printf("done.\r\n)");

    while (1)
        cmd_handler(&prog);

    return 0;
}

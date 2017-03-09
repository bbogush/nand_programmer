/* NAND */
#include "fsmc_nand.h"
/* SPL */
#include <stm32f10x.h>
/* USB */
#include <usb_lib.h>
#include <usb_pwr.h>
#include "hw_config.h"
/* STD */
#include <stdio.h>
#include <string.h>

#define NAND_PAGE_NUM      2
#define NAND_BUFFER_SIZE   (NAND_PAGE_NUM * NAND_PAGE_SIZE)

#define USB_BUF_SIZE 64

enum
{
    CMD_NAND_READ_ID = 0x00,
    CMD_NAND_ERASE   = 0x01,
    CMD_NAND_READ    = 0x02,
    CMD_NAND_WRITE   = 0x03,
};

typedef struct
{
    uint8_t code;
} cmd_t;

enum
{
    RESP_DATA   = 0x00,
    RESP_STATUS = 0x01,
};

typedef struct
{
    uint8_t code : 2;
    uint8_t data : 6;
} resp_header_t;

typedef enum
{
    STATUS_OK    = 0x00,
    STATUS_ERROR = 0x01,
} status_data_t;

typedef struct __attribute__((__packed__))
{
    resp_header_t header;
    NAND_IDTypeDef nand_id;
} resp_id_t;

NAND_ADDRESS nand_write_read_addr = { 0x00, 0x00, 0x00 };
uint8_t nand_write_buf[NAND_BUFFER_SIZE], nand_read_buf[NAND_BUFFER_SIZE];

extern __IO uint8_t Receive_Buffer[USB_BUF_SIZE];
extern __IO uint32_t Receive_length;
uint32_t packet_sent = 1;
uint32_t packet_receive = 1;
uint8_t usb_send_buf[USB_BUF_SIZE];

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

static void nand_init()
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

    NAND_Init();
}

static int make_status(uint8_t *buf, status_data_t status_data)
{
    resp_header_t status = { RESP_STATUS,  status_data };
    size_t len = sizeof(status);

    memcpy(buf, &status, len);

    return len;
}

void fill_buffer(uint8_t *buf, uint16_t buf_len, uint32_t offset)
{
    uint16_t i;

    /* Put in global buffer same values */
    for (i = 0; i < buf_len; i++)
        buf[i] = i + offset;
}

static int nand_read_id(uint8_t *buf, size_t buf_size)
{
    resp_id_t resp;
    size_t resp_len = sizeof(resp);

    if (buf_size < resp_len)
        return -1;

    resp.header.code = RESP_DATA;
    resp.header.data = resp_len - sizeof(resp.header);
    NAND_ReadID(&resp.nand_id);

    memcpy(buf, &resp, resp_len);

    return resp_len;
}

static int nand_erase(char *buf, size_t buf_size)
{
    uint32_t status;
    int len;

    /* Erase the NAND first Block */
    status = NAND_EraseBlock(nand_write_read_addr);

    len = snprintf(buf, buf_size, "0x%x\r\n", (unsigned int)status);
    if (len < 0 || len >= buf_size)
        return -1;

    len++;
    return len;
}

static int nand_write(char *buf, size_t buf_size)
{
    uint32_t status;
    int len;

    /* Write data to FSMC NAND memory */
    /* Fill the buffer to send */
    fill_buffer(nand_write_buf, NAND_BUFFER_SIZE , 0x66);

    status = NAND_WriteSmallPage(nand_write_buf, nand_write_read_addr,
        NAND_PAGE_NUM);
    len = snprintf(buf, buf_size, "0x%x\r\n", (unsigned int)status);
    if (len < 0 || len >= buf_size)
        return -1;

    len++;
    return len;
}

static int nand_read(char *buf, size_t buf_size)
{
    uint32_t status;
    int i, len, read_write_diff = 0;    

    /* Read back the written data */
    status = NAND_ReadSmallPage(nand_read_buf, nand_write_read_addr,
        NAND_PAGE_NUM);

    /* Verify the written data */
    for (i = 0; i < NAND_BUFFER_SIZE; i++)
    {
        if (nand_write_buf[i] != nand_read_buf[i])
            read_write_diff++;
    }

    len = snprintf(buf, buf_size, "0x%x %u\r\n", (unsigned int)status,
        read_write_diff);
    if (len < 0 || len >= buf_size)
        return -1;

    len++;
    return len;
}

static void usb_handler()
{
    int len;
    cmd_t cmd;

    if (bDeviceState != CONFIGURED)
        return;

    CDC_Receive_DATA();
    if (!Receive_length)
        return;

    cmd.code = Receive_Buffer[0];
    Receive_length = 0;

    switch (cmd.code)
    {
    case CMD_NAND_READ_ID:
        if ((len = nand_read_id(usb_send_buf, sizeof(usb_send_buf))) < 0)
            len = make_status(usb_send_buf, STATUS_ERROR);
        break;
    case CMD_NAND_ERASE:
        len = nand_erase((char *)usb_send_buf, sizeof(usb_send_buf));
        if (len < 0)
            return;
        break;
    case CMD_NAND_WRITE:
        len = nand_write((char *)usb_send_buf, sizeof(usb_send_buf));
        if (len < 0)
            return;
        break;
    case CMD_NAND_READ:
        len = nand_read((char *)usb_send_buf, sizeof(usb_send_buf));
        if (len < 0)
            return;
        break;
    default:
        return;
    }

    if (packet_sent)
        CDC_Send_DATA(usb_send_buf, len);
}

int main()
{
    jtag_init();

    usb_init();

    nand_init();

    while (1)
        usb_handler();

    return 0;
}

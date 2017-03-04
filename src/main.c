#include "stm3210e_eval_fsmc_nand.h"
/* SPL */
#include <stm32f10x.h>
/* USB */
#include <usb_lib.h>
#include <usb_pwr.h>
#include "hw_config.h"
/* STD */
#include <stdio.h>

#define PAGE_NUM      2
#define BUFFER_SIZE   (PAGE_NUM * NAND_PAGE_SIZE)

#define USB_BUF_SIZE 64

typedef enum
{
    CMD_NAND_READ_ID = 'i',
    CMD_NAND_ERASE   = 'e',
    CMD_NAND_READ    = 'r',
    CMD_NAND_WRITE   = 'w',
} cmd_t;

NAND_ADDRESS WriteReadAddr;
uint8_t TxBuffer[BUFFER_SIZE], RxBuffer[BUFFER_SIZE];
uint32_t PageNumber = 2, WriteReadStatus = 0, status= 0;
uint32_t j = 0;

uint32_t packet_sent=1;
uint32_t packet_receive=1;
extern __IO uint8_t Receive_Buffer[USB_BUF_SIZE];
extern __IO  uint32_t Receive_length ;
extern __IO  uint32_t length ;
uint8_t usb_send_buffer[USB_BUF_SIZE];


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

void Fill_Buffer(uint8_t *pBuffer, uint16_t BufferLenght, uint32_t Offset)
{
    uint16_t IndexTmp = 0;

    /* Put in global buffer same values */
    for (IndexTmp = 0; IndexTmp < BufferLenght; IndexTmp++ )
    {
        pBuffer[IndexTmp] = IndexTmp + Offset;
    }
}

static int nand_read_id(char *buf, size_t buf_size)
{
    NAND_IDTypeDef nand_id;
    int ret;

    NAND_ReadID(&nand_id);

    ret = snprintf(buf, buf_size, "0x%x 0x%x 0x%x 0x%x\r\n", nand_id.Maker_ID,
        nand_id.Device_ID, nand_id.Third_ID, nand_id.Fourth_ID);
    if (ret < 0 || ret >= buf_size)
        return -1;

    return ret;
}

/*static*/ void nand_erase()
{
    /* NAND memory address to write to */
    WriteReadAddr.Zone = 0x00;
    WriteReadAddr.Block = 0x00;
    WriteReadAddr.Page = 0x00;

    /* Erase the NAND first Block */
    status = NAND_EraseBlock(WriteReadAddr);
}

/*static*/ void nand_write()
{
    /* NAND memory address to write to */
    WriteReadAddr.Zone = 0x00;
    WriteReadAddr.Block = 0x00;
    WriteReadAddr.Page = 0x00;

    /* Write data to FSMC NAND memory */
    /* Fill the buffer to send */
    Fill_Buffer(TxBuffer, BUFFER_SIZE , 0x66);

    status = NAND_WriteSmallPage(TxBuffer, WriteReadAddr, PAGE_NUM);
}

/*static*/ void nand_read()
{
    /* NAND memory address to write to */
    WriteReadAddr.Zone = 0x00;
    WriteReadAddr.Block = 0x00;
    WriteReadAddr.Page = 0x00;

    /* Read back the written data */
    status = NAND_ReadSmallPage (RxBuffer, WriteReadAddr, PAGE_NUM);
   
    /* Verify the written data */
    for (j = 0; j < BUFFER_SIZE; j++)
    {
        if (TxBuffer[j] != RxBuffer[j])
        {
            WriteReadStatus++;
        }
    }
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

    cmd = Receive_Buffer[0];
    Receive_length = 0;

    switch (cmd)
    {
    case CMD_NAND_READ_ID:
        len = nand_read_id((char *)usb_send_buffer, sizeof(usb_send_buffer));
        if (len < 0)
            return;
        len++;
        break;
    default:
        return;
    }

    if (packet_sent)
        CDC_Send_DATA(usb_send_buffer, len);
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

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

NAND_IDTypeDef NAND_ID;
NAND_ADDRESS WriteReadAddr;
uint8_t TxBuffer[BUFFER_SIZE], RxBuffer[BUFFER_SIZE];
uint32_t PageNumber = 2, WriteReadStatus = 0, status= 0;
uint32_t j = 0;

uint32_t packet_sent=1;
uint32_t packet_receive=1;
extern __IO uint8_t Receive_Buffer[64];
extern __IO  uint32_t Receive_length ;
extern __IO  uint32_t length ;
uint8_t Send_Buffer[64];


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

static int nand_read_id(char *tx_buf, size_t buf_size)
{
    int ret;

    //NAND_ReadID(&NAND_ID);
    NAND_ID.Maker_ID = 0x12;
    NAND_ID.Device_ID = 0x34;
    NAND_ID.Third_ID = 0x56;
    NAND_ID.Fourth_ID = 0x78;

    ret = snprintf(tx_buf, buf_size, "0x%x 0x%x 0x%x 0x%x\r\n",
        NAND_ID.Maker_ID, NAND_ID.Device_ID, NAND_ID.Third_ID,
        NAND_ID.Fourth_ID);
    if (ret < 0 || ret >= buf_size)
        return -1;

    return ret;
}

static void nand_erase()
{
    /* NAND memory address to write to */
    WriteReadAddr.Zone = 0x00;
    WriteReadAddr.Block = 0x00;
    WriteReadAddr.Page = 0x00;

    /* Erase the NAND first Block */
    status = NAND_EraseBlock(WriteReadAddr);
}

static void nand_write()
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

static void nand_read()
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
    if (bDeviceState == CONFIGURED)
    {
        CDC_Receive_DATA();
        /*Check to see if we have data yet */
        if (Receive_length != 0)
        {
            int ret = 0;
            uint8_t cmd = Receive_Buffer[0];
 
            Receive_Buffer[0] = '\0';
 
            switch (cmd)
            {
            case 'i':
                ret = nand_read_id((char *)Receive_Buffer,
                    sizeof(Receive_Buffer));
                if (ret < 0)
                    return;
                break;
            case 'e':
                nand_erase();
                 break;
            case 'w':
                nand_write();
                break;
            case 'r':
                nand_read();
            default:
                break;
            }

            if (packet_sent == 1)
                CDC_Send_DATA ((unsigned char*)Receive_Buffer, ret + 1);

            Receive_length = 0;
        }
    }
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

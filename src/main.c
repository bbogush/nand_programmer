#include "stm3210e_eval_fsmc_nand.h"
/* SPL */
#include <stm32f10x.h>
#include <usb_lib.h>
#include <usb_pwr.h>
#include "hw_config.h"

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

void Fill_Buffer(uint8_t *pBuffer, uint16_t BufferLenght, uint32_t Offset)
{
    uint16_t IndexTmp = 0;

    /* Put in global buffer same values */
    for (IndexTmp = 0; IndexTmp < BufferLenght; IndexTmp++ )
    {
        pBuffer[IndexTmp] = IndexTmp + Offset;
    }
}

static void usb_periph_init()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /*Set PA11,12 as IN - USB_DM,DP*/ 
    RCC_AHBPeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure the EXTI line 18 connected internally to the USB IP */
    EXTI_ClearITPendingBit(EXTI_Line18);
    EXTI_InitStructure.EXTI_Line = EXTI_Line18;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);

    /* 2 bit for pre-emption priority, 2 bits for subpriority */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* Enable the USB interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
  
    /* Enable the USB Wake-up interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USBWakeUp_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_Init(&NVIC_InitStructure); 
}

int main()
{
    usb_periph_init();
    USB_Init();

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
    NAND_Init();
    NAND_ReadID(&NAND_ID);

    /* NAND memory address to write to */
    WriteReadAddr.Zone = 0x00;
    WriteReadAddr.Block = 0x00;
    WriteReadAddr.Page = 0x00;

    /* Erase the NAND first Block */
    status = NAND_EraseBlock(WriteReadAddr);

    /* Write data to FSMC NAND memory */
    /* Fill the buffer to send */
    Fill_Buffer(TxBuffer, BUFFER_SIZE , 0x66);

    status = NAND_WriteSmallPage(TxBuffer, WriteReadAddr, PAGE_NUM);

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

    while (1)
    {
        if (bDeviceState == CONFIGURED)
        {
            CDC_Receive_DATA();
            /*Check to see if we have data yet */
            if (Receive_length  != 0)
            {
                if (packet_sent == 1)
		{
                    CDC_Send_DATA ((unsigned char*)Receive_Buffer,
                        Receive_length);
                }
                Receive_length = 0;
            }
        }
    }

    return 0;
}

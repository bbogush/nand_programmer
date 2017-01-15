#include "stm3210e_eval_fsmc_nand.h"
/* SPL */
#include <stm32f10x.h>

#define PAGE_NUM      2
#define BUFFER_SIZE   (PAGE_NUM * NAND_PAGE_SIZE)

NAND_IDTypeDef NAND_ID;
NAND_ADDRESS WriteReadAddr;
uint8_t TxBuffer[BUFFER_SIZE], RxBuffer[BUFFER_SIZE];
uint32_t PageNumber = 2, WriteReadStatus = 0, status= 0;
uint32_t j = 0;

void Fill_Buffer(uint8_t *pBuffer, uint16_t BufferLenght, uint32_t Offset)
{
    uint16_t IndexTmp = 0;

    /* Put in global buffer same values */
    for (IndexTmp = 0; IndexTmp < BufferLenght; IndexTmp++ )
    {
        pBuffer[IndexTmp] = IndexTmp + Offset;
    }
}

int main()
{
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

    while(1);

    return 0;
}

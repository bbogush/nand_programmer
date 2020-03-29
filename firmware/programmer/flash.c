/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include <stm32f10x.h>

int flash_page_erase(uint32_t page_addr)
{
    FLASH_Status status;

    __disable_irq();
    FLASH_Unlock();

    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR |
        FLASH_FLAG_WRPRTERR);	

    status = FLASH_ErasePage(page_addr);

    FLASH_Lock();
    __enable_irq();

    return status != FLASH_COMPLETE ? -1 : 0;
}

int flash_write(uint32_t addr, uint8_t *data, uint32_t data_len)
{
    uint32_t word, count, i;
    int ret = -1;

    __disable_irq();
    FLASH_Unlock();

    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR |
        FLASH_FLAG_WRPRTERR);	

    count = data_len / 4;
    if (data_len % 4)
        count++;
    for (i = 0 ; i < count ; i++)
    {
        word = *((uint32_t *)data + i);
        if (FLASH_ProgramWord(addr, word) != FLASH_COMPLETE)
            goto Exit;

        addr += 4;
    }

    ret = data_len;
Exit:
    FLASH_Lock();
    __enable_irq();

    return ret;
}

int flash_read(uint32_t addr, uint8_t *data, uint32_t data_len)
{
    uint32_t i;

    for(i = 0; i < data_len; i++)
        data[i] = *(uint8_t *) (addr + i);

    return i;
}

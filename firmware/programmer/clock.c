/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "clock.h"
#include <stm32f10x.h>

bool is_external_clock_avail()
{
    return (RCC->CR & RCC_CR_HSERDY) != RESET;
}

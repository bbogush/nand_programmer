/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "led.h"

void led_init()
{
    GPIO_InitTypeDef led_gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, 1);

    led_gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    led_gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;    
    led_gpio.GPIO_Speed = GPIO_Speed_2MHz;

    GPIO_Init(GPIOA, &led_gpio);

    GPIO_ResetBits(GPIOA, GPIO_Pin_0 | GPIO_Pin_1);
}

static void led_set(GPIO_TypeDef *gpiox, uint16_t pin, bool on)
{
    if (on)
        GPIO_SetBits(gpiox, pin);
    else
        GPIO_ResetBits(gpiox, pin);
}

void led_wr_set(bool on)
{
    led_set(GPIOA, GPIO_Pin_0, on);
}

void led_rd_set(bool on)
{
    led_set(GPIOA, GPIO_Pin_1, on);
}


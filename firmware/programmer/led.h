/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef _LED_H_
#define _LED_H_

#include <stdbool.h>

void led_init();
void led_wr_set(bool on);
void led_rd_set(bool on);

#endif


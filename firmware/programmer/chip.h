/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef _CHIP_H_
#define _CHIP_H_

typedef struct __attribute__((__packed__))
{
    uint8_t maker_id;
    uint8_t device_id;
    uint8_t third_id;
    uint8_t fourth_id;
    uint8_t fifth_id;
} chip_id_t;

#endif /* _CHIP_H_ */

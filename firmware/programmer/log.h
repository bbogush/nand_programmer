/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>

#ifdef DEBUG
    #define DEBUG_PRINT printf
#else
    #define DEBUG_PRINT(...)
#endif
#define ERROR_PRINT(fmt, args...) printf("ERROR: "fmt, ## args)

#endif

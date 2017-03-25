/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include <sys/reent.h>
#include <sys/types.h>

/* Stub for sprintf */
caddr_t _sbrk_r(struct _reent *r, int incr)
{
    return 0;
}

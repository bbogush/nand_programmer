/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include <sys/reent.h>
#include <sys/types.h>
#include <sys/stat.h>

int __io_putchar(int ch);

caddr_t _sbrk_r(struct _reent *r, int incr)
{
    extern char _ebss; /* defined by linker */
    static char *heap_end;
    char *prev_heap_end; 
    register char *stack asm("sp");

    if (!heap_end)
        heap_end = &_ebss;
    prev_heap_end = heap_end;

    if (heap_end + incr > stack)
         return (caddr_t)-1;

    heap_end += incr;
    return (caddr_t)prev_heap_end;
}

int _write(int file, char *ptr, int len)
{
    int i;

    for (i = 0; i < len; i++)
        __io_putchar( *ptr++ );

    return len;
}

int _close(int file)
{
    return -1;
}

int _fstat(int file, struct stat *st)
{
    return -1;
}

int _isatty(int file)
{
    return 0;
}

int _lseek(int file, int ptr, int dir)
{
    return -1;
}

int _read(int file, char *ptr, int len)
{
    return 0;
}

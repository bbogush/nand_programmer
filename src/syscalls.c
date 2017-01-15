#include <sys/reent.h>
#include <sys/types.h>

/* Stub for sprintf */
caddr_t _sbrk_r(struct _reent *r, int incr)
{
    return 0;
}

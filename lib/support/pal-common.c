#include "sysmel/pal.h"
#include <stdlib.h>

SYSMEL_PAL_EXTERN_C void sysmel_pal_abort(void)
{
    abort();
}

SYSMEL_PAL_EXTERN_C void* sysmel_pal_malloc(size_t size)
{
    return malloc(size);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_free(void *pointer)
{
    return free(pointer);
}

#include "sysmel/pal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

SYSMEL_PAL_EXTERN_C double sysmel_pal_parseFloat64(size_t stringSize, const char *string)
{
    char *buffer = (char*)malloc(stringSize + 1);
    memcpy(buffer, string, stringSize);
    buffer[stringSize] = 0;

    double result = atof(buffer);
    free(buffer);
    return result;
}

SYSMEL_PAL_EXTERN_C char *sysmel_pal_float64ToString(double value)
{
    char buffer[32] = {0};
    int stringSize = snprintf(buffer, sizeof(buffer), "%g", value);
    if(stringSize < 0)
    {
        char *result = (char*)malloc(4);
        memcpy(result, "0.0", 4);
        return result;
    }
    else
    {
        char *result = (char*)malloc(stringSize + 1);
        memcpy(result, buffer, stringSize);
        result[stringSize] = 0;
        return result;
    }
}

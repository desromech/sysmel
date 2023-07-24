#include "sysmel/pal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef USE_BOEHM_GC
#include <gc.h>
#endif

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
    free(pointer);
}

#ifdef USE_BOEHM_GC
SYSMEL_PAL_EXTERN_C void* sysmel_pal_gcalloc(size_t size)
{
    return GC_malloc(size);
}
#else
static const size_t ChunkSize = 4<<20;
static uint8_t *currentChunk;
static size_t currentChunkSize;

SYSMEL_PAL_EXTERN_C void* sysmel_pal_gcalloc(size_t size)
{
    size_t alignedSize = (size + 15) & (-(size_t)16);
    if(alignedSize >= ChunkSize / 4)
        return (uint8_t*)malloc(alignedSize);

    if(!currentChunk || currentChunkSize + alignedSize > ChunkSize)
    {
        currentChunk = (uint8_t*)malloc(ChunkSize);
        currentChunkSize = 0;
    }

    uint8_t *result = currentChunk + currentChunkSize;
    currentChunkSize += alignedSize;
    return result;
}
#endif

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

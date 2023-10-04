#ifndef SYSBVM_DYNARRAY_CONTEXT_H
#define SYSBVM_DYNARRAY_CONTEXT_H

#include <stddef.h>
#include <stdint.h>
#include "common.h"

typedef struct sysbvm_dynarray_s
{
    size_t entrySize;
    size_t size;
    size_t capacity;
    uint8_t *data;
} sysbvm_dynarray_t;

SYSBVM_API void sysbvm_dynarray_initialize(sysbvm_dynarray_t *dynarray, size_t entrySize, size_t initialCapacity);
SYSBVM_API size_t sysbvm_dynarray_addAll(sysbvm_dynarray_t *dynarray, size_t entryCount, const void *newEntries);
SYSBVM_API size_t sysbvm_dynarray_add(sysbvm_dynarray_t *dynarray, const void *newEntry);
SYSBVM_API void sysbvm_dynarray_destroy(sysbvm_dynarray_t *dynarray);

#define sysbvm_dynarray_entryOfTypeAt(dynarray, entryType, index) (((entryType*)(dynarray).data) + index)

#endif //SYSBVM_DYNARRAY_CONTEXT_H
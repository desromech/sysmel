#include "dynarray.h"
#include <malloc.h>
#include <string.h>

void sysbvm_dynarray_initialize(sysbvm_dynarray_t *dynarray, size_t entrySize, size_t initialCapacity)
{
    dynarray->entrySize = entrySize;
    dynarray->size = 0;
    dynarray->capacity = initialCapacity;
    dynarray->data = initialCapacity > 0 ? (uint8_t*)malloc(initialCapacity * entrySize) : NULL;
}

size_t sysbvm_dynarray_addAll(sysbvm_dynarray_t *dynarray, size_t entryCount, const void *newEntries)
{
    size_t requiredCapacity = dynarray->size + entryCount;
    if(requiredCapacity > dynarray->capacity)
    {
        size_t newCapacity = dynarray->capacity;
        if(newCapacity < 16)
            newCapacity = 16;

        while(newCapacity < requiredCapacity)
            newCapacity *= 2;

        uint8_t *newStorage = (uint8_t*)malloc(dynarray->entrySize * newCapacity);
        memcpy(newStorage, dynarray->data, dynarray->entrySize * dynarray->size);
        free(dynarray->data);

        dynarray->capacity = newCapacity;
        dynarray->data = newStorage;
    }

    memcpy(dynarray->data + dynarray->size*dynarray->entrySize, newEntries, entryCount*dynarray->entrySize);
    dynarray->size += entryCount;
    return dynarray->size;
}

size_t sysbvm_dynarray_add(sysbvm_dynarray_t *dynarray, const void *newEntry)
{
    return sysbvm_dynarray_addAll(dynarray, 1, newEntry);
}

void sysbvm_dynarray_destroy(sysbvm_dynarray_t *dynarray)
{
    free(dynarray->data);
}

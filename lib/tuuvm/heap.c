#include "internal/heap.h"
#include <stdlib.h>
#include <string.h>
/**
 * Allocates a byte tuple with the specified size.
 */
TUUVM_API tuvvm_tuple_t *tuuvm_heap_allocateByteTuple(tuuvm_heap_t *heap, size_t byteSize)
{
    size_t allocationSize = sizeof(tuvvm_tuple_t) + byteSize;
    tuvvm_tuple_t *result = (tuvvm_tuple_t *)calloc(1, allocationSize);
    result->header.typePointerAndFlags = TUUVM_TUPLE_BYTES_BIT;
    result->header.objectSize = byteSize;
    return result;
}

/**
 * Allocates a pointer tuple with the specified slot count.
 */
TUUVM_API tuvvm_tuple_t *tuuvm_heap_allocatePointerTuple(tuuvm_heap_t *heap, size_t slotCount)
{
    size_t allocationSize = sizeof(tuvvm_tuple_t) + slotCount*sizeof(tuvvm_tuple_t*);
    tuvvm_tuple_t *result = (tuvvm_tuple_t *)calloc(1, allocationSize);
    result->header.typePointerAndFlags = TUUVM_TUPLE_BYTES_BIT;
    return result;
}

#include "tuuvm/heap.h"

/**
 * Allocates a byte tuple with the specified size.
 */
TUUVM_API tuvvm_tuple_t *tuuvm_heap_allocateByteTuple(tuuvm_heap_t *heap, size_t byteSize)
{
    return TUUVM_NULL_TUPLE;
}

/**
 * Allocates a pointer tuple with the specified slot count.
 */
TUUVM_API tuvvm_tuple_t *tuuvm_heap_allocatePointerTuple(tuuvm_heap_t *heap, size_t slotCount)
{
    return TUUVM_NULL_TUPLE;
}

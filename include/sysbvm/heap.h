#ifndef SYSBVM_HEAP_H
#define SYSBVM_HEAP_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_heap_s sysbvm_heap_t;

/**
 * Allocates a byte tuple with the specified size.
 */
SYSBVM_API sysbvm_object_tuple_t *sysbvm_heap_allocateByteTuple(sysbvm_heap_t *heap, size_t byteSize);

/**
 * Allocates a pointer tuple with the specified slot count.
 */
SYSBVM_API sysbvm_object_tuple_t *sysbvm_heap_allocatePointerTuple(sysbvm_heap_t *heap, size_t slotCount);

/**
 * Performs a shallow copy of the specified tuple.
 */
SYSBVM_API sysbvm_object_tuple_t *sysbvm_heap_shallowCopyTuple(sysbvm_heap_t *heap, sysbvm_object_tuple_t *tupleToCopy);

#endif //SYSBVM_HEAP_H

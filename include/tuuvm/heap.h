#ifndef TUUVM_HEAP_H
#define TUUVM_HEAP_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_heap_s tuuvm_heap_t;

/**
 * Allocates a byte tuple with the specified size.
 */
TUUVM_API tuuvm_tuple_t *tuuvm_heap_allocateByteTuple(tuuvm_heap_t *heap, size_t byteSize);

/**
 * Allocates a pointer tuple with the specified slot count.
 */
TUUVM_API tuuvm_tuple_t *tuuvm_heap_allocatePointerTuple(tuuvm_heap_t *heap, size_t slotCount);

#endif //TUUVM_HEAP_H

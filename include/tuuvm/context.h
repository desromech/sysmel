#ifndef TUUVM_CONTEXT_H
#define TUUVM_CONTEXT_H

#pragma once

#include "common.h"
#include "heap.h"
#include "tuple.h"

typedef struct tuvvm_context_s tuuvm_context_t;
typedef struct tuuvm_heap_s tuuvm_heap_t;

/**
 * Creates a new context.
 */
TUUVM_API tuuvm_context_t *tuuvm_context_create(void);

/**
 * Creates a new context.
 */
TUUVM_API void tuuvm_context_destroy(tuuvm_context_t *context);

/**
 * Gets the heap present in a context.
 */
TUUVM_API tuuvm_heap_t *tuuvm_context_getHeap(tuuvm_context_t *context);

/**
 * Allocates a byte tuple with the specified size.
 */
TUUVM_API tuuvm_tuple_t *tuuvm_context_allocateByteTuple(tuuvm_context_t *context, size_t byteSize);

/**
 * Allocates a pointer tuple with the specified slot count.
 */
TUUVM_API tuuvm_tuple_t *tuuvm_context_allocatePointerTuple(tuuvm_context_t *context, size_t slotCount);

#endif //TUUVM_CONTEXT_H

#ifndef TUUVM_CONTEXT_H
#define TUUVM_CONTEXT_H

#pragma once

#include "common.h"
#include "heap.h"
#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;
typedef struct tuuvm_heap_s tuuvm_heap_t;

typedef tuuvm_tuple_t (*tuuvm_functionEntryPoint_t)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments);


/**
 * Creates a new context.
 */
TUUVM_API tuuvm_context_t *tuuvm_context_create(void);

/**
 * Creates a context by loading it from an image.
 */
TUUVM_API tuuvm_context_t *tuuvm_context_loadImageFromFileNamed(const char *filename);

/**
 * Saves a context into an image.
 */
TUUVM_API void tuuvm_context_saveImageToFileNamed(tuuvm_context_t *context, const char *filename);


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
TUUVM_API tuuvm_object_tuple_t *tuuvm_context_allocateByteTuple(tuuvm_context_t *context, tuuvm_tuple_t type, size_t byteSize);

/**
 * Allocates a pointer tuple with the specified slot count.
 */
TUUVM_API tuuvm_object_tuple_t *tuuvm_context_allocatePointerTuple(tuuvm_context_t *context, tuuvm_tuple_t type, size_t slotCount);

/**
 * Performs a shallow copy of a tuple
 */
TUUVM_API tuuvm_tuple_t tuuvm_context_shallowCopy(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Creates an intrinsic type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_context_createIntrinsicType(tuuvm_context_t *context, const char *name, tuuvm_tuple_t supertype, ...);

/**
 * Sets an intrinsic symbol binding.
 */
TUUVM_API void tuuvm_context_setIntrinsicSymbolBindingValue(tuuvm_context_t *context, tuuvm_tuple_t symbol, tuuvm_tuple_t binding);

/**
 * Sets an intrinsic symbol binding with primitive function.
 */
TUUVM_API tuuvm_tuple_t tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(tuuvm_context_t *context, const char *symbolString, size_t argumentCount, size_t flags, void *userdata, tuuvm_functionEntryPoint_t entryPoint);

/**
 * Sets an intrinsic symbol binding with primitive function.
 */
TUUVM_API tuuvm_tuple_t tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(tuuvm_context_t *context, const char *symbolString, tuuvm_tuple_t ownerClass, const char *selectorString, size_t argumentCount, size_t flags, void *userdata, tuuvm_functionEntryPoint_t entryPoint);

/**
 * Registers a function in the global primitive table.
 */
TUUVM_API void tuuvm_primitiveTable_registerFunction(tuuvm_functionEntryPoint_t primitiveEntryPoint, const char *primitiveName);

#endif //TUUVM_CONTEXT_H

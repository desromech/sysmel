#ifndef SYSBVM_CONTEXT_H
#define SYSBVM_CONTEXT_H

#pragma once

#include "common.h"
#include "heap.h"
#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;
typedef struct sysbvm_heap_s sysbvm_heap_t;

typedef sysbvm_tuple_t (*sysbvm_functionEntryPoint_t)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments);

typedef struct sysbvm_contextCreationOptions_s
{
    uint32_t targetWordSize;
    const char *buildArchitectureName;
    const char *buildVendorName;
    const char *buildOSName;
    const char *buildAbiName;
    const char *buildObjectFileName;
    const char *buildDebugInformationFormatName;
    const char *buildExceptionHandlingTableFormatName;
    const char *hostArchitectureName;
    const char *hostVendorName;
    const char *hostOSName;
    const char *hostAbiName;
    const char *hostObjectFileName;
    const char *hostDebugInformationFormatName;
    const char *hostExceptionHandlingTableFormatName;
    const char *targetArchitectureName;
    const char *targetVendorName;
    const char *targetOSName;
    const char *targetAbiName;
    const char *targetObjectFileName;
    const char *targetDebugInformationFormatName;
    const char *targetExceptionHandlingTableFormatName;
    bool nojit;
} sysbvm_contextCreationOptions_t;

/**
 * Creates a new context.
 */
SYSBVM_API sysbvm_context_t *sysbvm_context_create(void);

/**
 * Creates a new context.
 */
SYSBVM_API sysbvm_context_t *sysbvm_context_createWithOptions(sysbvm_contextCreationOptions_t *contextOptions);

/**
 * Creates a context by loading it from an image.
 */
SYSBVM_API sysbvm_context_t *sysbvm_context_loadImageFromFileNamed(const char *filename);

/**
 * Saves a context into an image.
 */
SYSBVM_API void sysbvm_context_saveImageToFileNamed(sysbvm_context_t *context, const char *filename);


/**
 * Creates a new context.
 */
SYSBVM_API void sysbvm_context_destroy(sysbvm_context_t *context);

/**
 * Gets the heap present in a context.
 */
SYSBVM_API sysbvm_heap_t *sysbvm_context_getHeap(sysbvm_context_t *context);

/**
 * Allocates a byte tuple with the specified size.
 */
SYSBVM_API sysbvm_object_tuple_t *sysbvm_context_allocateByteTuple(sysbvm_context_t *context, sysbvm_tuple_t type, size_t byteSize);

/**
 * Allocates a pointer tuple with the specified slot count.
 */
SYSBVM_API sysbvm_object_tuple_t *sysbvm_context_allocatePointerTuple(sysbvm_context_t *context, sysbvm_tuple_t type, size_t slotCount);

/**
 * Performs a shallow copy of a tuple
 */
SYSBVM_API sysbvm_tuple_t sysbvm_context_shallowCopy(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Creates an intrinsic type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_context_createIntrinsicType(sysbvm_context_t *context, const char *name, sysbvm_tuple_t supertype, ...);

/**
 * Sets an intrinsic symbol binding.
 */
SYSBVM_API void sysbvm_context_setIntrinsicSymbolBindingValue(sysbvm_context_t *context, sysbvm_tuple_t symbol, sysbvm_tuple_t binding);

/**
 * Sets an intrinsic symbol binding.
 */
SYSBVM_API void sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(sysbvm_context_t *context, const char *symbolName, sysbvm_tuple_t binding);

/**
 * Sets an intrinsic symbol binding with primitive function.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(sysbvm_context_t *context, const char *symbolString, size_t argumentCount, sysbvm_bitflags_t flags, void *userdata, sysbvm_functionEntryPoint_t entryPoint);

/**
 * Sets an intrinsic symbol binding with primitive function.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(sysbvm_context_t *context, const char *symbolString, sysbvm_tuple_t ownerClass, const char *selectorString, size_t argumentCount, sysbvm_bitflags_t flags, void *userdata, sysbvm_functionEntryPoint_t entryPoint);

/**
 * Registers a function in the global primitive table.
 */
SYSBVM_API void sysbvm_primitiveTable_registerFunction(sysbvm_functionEntryPoint_t primitiveEntryPoint, const char *primitiveName);

#endif //SYSBVM_CONTEXT_H

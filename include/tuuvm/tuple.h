#ifndef TUUVM_TUPLE_H
#define TUUVM_TUPLE_H

#pragma once

#include "common.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct tuvvm_context_s tuuvm_context_t;

/**
 * A tuple object pointer.
 */
typedef uintptr_t tuuvm_tuple_t;

#define TUUVM_TUPLE_TYPE_POINTER_MASK ((uintptr_t)-16)
#define TUUVM_TUPLE_FLAGS_MASK ((uintptr_t)15)

#define TUUVM_TUPLE_GC_COLOR_MASK ((uintptr_t)-4)
#define TUUVM_TUPLE_BYTES_BIT ((uintptr_t)4)
#define TUUVM_TUPLE_IMMUTABLE_BIT ((uintptr_t)8)

#define TUUVM_TUPLE_TAG_BIT_COUNT 4
#define TUUVM_TUPLE_TAG_BIT_MASK 15

#define TUUVM_TUPLE_TAG_POINTER 0

#define TUUVM_CAST_OOP_TO_OBJECT_TUPLE(oop) ((tuuvm_object_tuple_t*)oop)
/**
 * Tuple header that describes its content.
 */
typedef struct tuuvm_tuple_header_s
{
    uintptr_t typePointerAndFlags;
    size_t objectSize;
} tuuvm_tuple_header_t;

/**
 * A tuple can contain pointers and/or bytes.
 */
typedef struct tuuvm_object_tuple_s
{
    tuuvm_tuple_header_t header;

    union
    {
        tuuvm_tuple_t pointers[1];
        uint8_t bytes[1];
    };
} tuuvm_object_tuple_t;

#define TUUVM_NULL_TUPLE ((tuuvm_tuple_t)0)

/**
 * Is this a pointer tuple?
 */
static inline size_t tuuvm_tuple_isPointer(tuuvm_tuple_t tuple)
{
    return (tuple & TUUVM_TUPLE_TAG_BIT_MASK) == TUUVM_TUPLE_TAG_POINTER;
}

/**
 * Is this non-null pointer tuple?
 */
static inline size_t tuuvm_tuple_isNonNullPointer(tuuvm_tuple_t tuple)
{
    return tuple != 0 && (tuple & TUUVM_TUPLE_TAG_BIT_MASK) == TUUVM_TUPLE_TAG_POINTER;
}

/**
 * Is this an immediate tuple=
 */
static inline size_t tuuvm_tuple_isImmediate(tuuvm_tuple_t tuple)
{
    return (tuple & TUUVM_TUPLE_TAG_BIT_MASK) != TUUVM_TUPLE_TAG_POINTER;
}

/**
 * Gets the size in bytes of the specified tuple.
 */
static inline size_t tuuvm_tuple_getSizeInBytes(tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isNonNullPointer(tuple) ? TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.objectSize : 0;
}

TUUVM_API tuuvm_object_tuple_t *tuuvm_tuple_getImmediateTypeWithTag(tuuvm_context_t *context, size_t immediateTypeTag);

/**
 * Gets the type tuple of the specified tuple.
 */
static inline tuuvm_object_tuple_t *tuuvm_tuple_getType(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    if(tuuvm_tuple_isNonNullPointer(tuple))
        return (tuuvm_object_tuple_t*)(TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.typePointerAndFlags & TUUVM_TUPLE_TYPE_POINTER_MASK);
    else
        return tuuvm_tuple_getImmediateTypeWithTag(context, tuple & TUUVM_TUPLE_TAG_BIT_MASK);
        
}

/**
 * Is this a bytes tuple?
 */
static inline bool tuuvm_tuple_isBytes(tuuvm_tuple_t tuple)
{
    return (TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.typePointerAndFlags & TUUVM_TUPLE_BYTES_BIT) != 0;
}

/**
 * Is this an immutable tuple?
 */
static inline bool tuuvm_tuple_isImmutable(tuuvm_tuple_t tuple)
{
    return !tuuvm_tuple_isNonNullPointer(tuple) || (TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.typePointerAndFlags & TUUVM_TUPLE_IMMUTABLE_BIT) != 0;
}

#endif //TUUVM_TUPLE_H

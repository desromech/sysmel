#ifndef TUUVM_TUPLE_H
#define TUUVM_TUPLE_H

#pragma once

#include "common.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct tuuvm_context_s tuuvm_context_t;

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

#define TUUVM_CAST_OOP_TO_OBJECT_TUPLE(oop) ((tuuvm_object_tuple_t*)oop)

/**
 * The different immediate pointer tags.
 */ 
typedef enum tuuvm_tuple_tag_e
{
    TUUVM_TUPLE_TAG_POINTER = 0,
    TUUVM_TUPLE_TAG_NIL = 0,

    TUUVM_TUPLE_TAG_INTEGER = 1,
    TUUVM_TUPLE_TAG_CHAR8 = 2,
    TUUVM_TUPLE_TAG_UINT8 = 3,
    TUUVM_TUPLE_TAG_INT8 = 4,

    TUUVM_TUPLE_TAG_CHAR16 = 5,
    TUUVM_TUPLE_TAG_UINT16 = 6,
    TUUVM_TUPLE_TAG_INT16 = 7,

    TUUVM_TUPLE_TAG_CHAR32 = 8,
    TUUVM_TUPLE_TAG_UINT32 = 9,
    TUUVM_TUPLE_TAG_INT32 = 10,

    TUUVM_TUPLE_TAG_UINT64 = 11,
    TUUVM_TUPLE_TAG_INT64 = 12,

    TUUVM_TUPLE_TAG_FLOAT = 13,
    TUUVM_TUPLE_TAG_DOUBLE = 14,

    TUUVM_TUPLE_TAG_TRIVIAL = 15,

    TUUVM_TUPLE_TAG_COUNT = 16
} tuuvm_tuple_tag_t;

/**
 * The indices for the different immediate trivial values.
 */ 
typedef enum tuuvm_tuple_immediate_trivial_index_e
{
    TUUVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_FALSE = 0,
    TUUVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_TRUE,
    TUUVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_VOID,
    TUUVM_TUPLE_IMMEDIATE_TRIVIAL_COUNT,
} tuuvm_tuple_immediate_trivial_index_e;

#define TUUVM_TUPLE_MAKE_IMMEDIATE_TRIVIAL_WITH_INDEX(immediateTrivialIndex) (immediateTrivialIndex << TUUVM_TUPLE_TAG_BIT_COUNT) | 
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

#define TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(structureType) ((sizeof(structureType) - sizeof(tuuvm_tuple_header_t)) / sizeof(tuuvm_tuple_t))
/**
 * Is this a pointer tuple?
 */
TUUVM_INLINE size_t tuuvm_tuple_isPointer(tuuvm_tuple_t tuple)
{
    return (tuple & TUUVM_TUPLE_TAG_BIT_MASK) == TUUVM_TUPLE_TAG_POINTER;
}

/**
 * Is this non-null pointer tuple?
 */
TUUVM_INLINE size_t tuuvm_tuple_isNonNullPointer(tuuvm_tuple_t tuple)
{
    return tuple != 0 && (tuple & TUUVM_TUPLE_TAG_BIT_MASK) == TUUVM_TUPLE_TAG_POINTER;
}

/**
 * Is this an immediate tuple=
 */
TUUVM_INLINE size_t tuuvm_tuple_isImmediate(tuuvm_tuple_t tuple)
{
    return (tuple & TUUVM_TUPLE_TAG_BIT_MASK) != TUUVM_TUPLE_TAG_POINTER;
}

/**
 * Gets the size in bytes of the specified tuple.
 */
TUUVM_INLINE size_t tuuvm_tuple_getSizeInBytes(tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isNonNullPointer(tuple) ? TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.objectSize : 0;
}

/**
 * Retrieves an immediate tuple type corresponding to a specific tag.
 */
TUUVM_API tuuvm_tuple_t tuuvm_tuple_getImmediateTypeWithTag(tuuvm_context_t *context, size_t immediateTypeTag);

/**
 * Retrieves an immediate trivial tuple corresponding to a specific index.
 */
TUUVM_API tuuvm_tuple_t tuuvm_tuple_getImmediateTrivialTypeWithIndex(tuuvm_context_t *context, size_t immediateTrivialIndex);

/**
 * Sets the type of an object tuple
 */
TUUVM_INLINE void tuuvm_tuple_setType(tuuvm_object_tuple_t *objectTuple, tuuvm_tuple_t newType)
{
    objectTuple->header.typePointerAndFlags = (objectTuple->header.typePointerAndFlags & TUUVM_TUPLE_FLAGS_MASK) | newType;
}

/**
 * Gets the type tuple of the specified tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_getType(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    if(tuuvm_tuple_isNonNullPointer(tuple))
    {
        return TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.typePointerAndFlags & TUUVM_TUPLE_TYPE_POINTER_MASK;
    }
    else
    {
        size_t typeTag = tuple & TUUVM_TUPLE_TAG_BIT_MASK;
        if(typeTag != TUUVM_TUPLE_TAG_TRIVIAL)
            return tuuvm_tuple_getImmediateTypeWithTag(context, typeTag);
        else
            return tuuvm_tuple_getImmediateTrivialTypeWithIndex(context, tuple >> TUUVM_TUPLE_TAG_BIT_COUNT);
    }
}

/**
 * Is this a bytes tuple?
 */
TUUVM_INLINE bool tuuvm_tuple_isBytes(tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isNonNullPointer(tuple) && (TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.typePointerAndFlags & TUUVM_TUPLE_BYTES_BIT) != 0;
}

/**
 * Is this an immutable tuple?
 */
TUUVM_INLINE bool tuuvm_tuple_isImmutable(tuuvm_tuple_t tuple)
{
    return !tuuvm_tuple_isNonNullPointer(tuple) || (TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.typePointerAndFlags & TUUVM_TUPLE_IMMUTABLE_BIT) != 0;
}

#endif //TUUVM_TUPLE_H

#ifndef TUUVM_TUPLE_H
#define TUUVM_TUPLE_H

#pragma once

#ifdef MSC_VER
#endif

#include "common.h"
#include "hash.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct tuuvm_context_s tuuvm_context_t;

typedef uint8_t tuuvm_char8_t;
typedef uint16_t tuuvm_char16_t;
typedef uint32_t tuuvm_char32_t;

typedef float tuuvm_float32_t;
typedef double tuuvm_float64_t;

/**
 * A tuple object pointer.
 */
typedef uintptr_t tuuvm_tuple_t;

/**
 * A tuple object signed pointer.
 */
typedef intptr_t tuuvm_stuple_t;

#define TUUVM_TUPLE_TYPE_POINTER_MASK ((uintptr_t)-16)
#define TUUVM_TUPLE_FLAGS_MASK ((uintptr_t)15)

#define TUUVM_TUPLE_GC_COLOR_MASK ((uintptr_t)3)
#define TUUVM_TUPLE_BYTES_BIT ((uintptr_t)4)
#define TUUVM_TUPLE_NEEDS_FINALIZATION ((uintptr_t)8)

#define TUUVM_TUPLE_TAG_BIT_COUNT 4
#define TUUVM_TUPLE_TAG_BIT_MASK 15

#define TUUVM_CAST_OOP_TO_OBJECT_TUPLE(oop) ((tuuvm_object_tuple_t*)oop)

#define TUUVM_TUPLE_BIT_COUNT ((sizeof(tuuvm_tuple_t)*8))
#define TUUVM_IMMEDIATE_BIT_COUNT (TUUVM_TUPLE_BIT_COUNT - TUUVM_TUPLE_TAG_BIT_COUNT)
#define TUUVM_TUPLE_IMMEDIATE_VALUE_BIT_MASK (((tuuvm_tuple_t)1 << TUUVM_IMMEDIATE_BIT_COUNT) - 1)

#define TUUVM_IMMEDIATE_UINT_MIN ((tuuvm_tuple_t)0)
#define TUUVM_IMMEDIATE_UINT_MAX ( ((tuuvm_tuple_t)1 << TUUVM_IMMEDIATE_BIT_COUNT) - 1 )

#define TUUVM_IMMEDIATE_INT_MIN ( -((tuuvm_stuple_t)1 << (TUUVM_IMMEDIATE_BIT_COUNT - 1)) )
#define TUUVM_IMMEDIATE_INT_MAX ( ((tuuvm_stuple_t)1 << (TUUVM_IMMEDIATE_BIT_COUNT - 1)) - 1 )

/**
 * The different immediate pointer tags.
 */ 
typedef enum tuuvm_tuple_tag_e
{
    TUUVM_TUPLE_TAG_POINTER = 0,
    TUUVM_TUPLE_TAG_NIL = 0,

    TUUVM_TUPLE_TAG_INTEGER = 1,
    TUUVM_TUPLE_TAG_INT8 = 2,
    TUUVM_TUPLE_TAG_INT16 = 3,
    TUUVM_TUPLE_TAG_INT32 = 4,
    TUUVM_TUPLE_TAG_INT64 = 5,

    TUUVM_TUPLE_TAG_CHAR8 = 6,
    TUUVM_TUPLE_TAG_UINT8 = 7,

    TUUVM_TUPLE_TAG_CHAR16 = 8,
    TUUVM_TUPLE_TAG_UINT16 = 9,

    TUUVM_TUPLE_TAG_CHAR32 = 10,
    TUUVM_TUPLE_TAG_UINT32 = 11,

    TUUVM_TUPLE_TAG_UINT64 = 12,

    TUUVM_TUPLE_TAG_FLOAT32 = 13,
    TUUVM_TUPLE_TAG_FLOAT64 = 14,

    TUUVM_TUPLE_TAG_TRIVIAL = 15,

    TUUVM_TUPLE_TAG_SIGNED_START = TUUVM_TUPLE_TAG_INTEGER,
    TUUVM_TUPLE_TAG_SIGNED_END = TUUVM_TUPLE_TAG_INT64,

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
    TUUVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_HASHTABLE_EMPTY_ELEMENT,

    TUUVM_TUPLE_IMMEDIATE_TRIVIAL_COUNT,
} tuuvm_tuple_immediate_trivial_index_e;

#define TUUVM_TUPLE_MAKE_IMMEDIATE_TRIVIAL_WITH_INDEX(immediateTrivialIndex) ((immediateTrivialIndex << TUUVM_TUPLE_TAG_BIT_COUNT) | TUUVM_TUPLE_TAG_TRIVIAL)
/**
 * Tuple header that describes its content.
 */
typedef struct tuuvm_tuple_header_s
{
    tuuvm_tuple_t forwardingPointer; // Used by the moving GC
    size_t identityHash;
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
#define TUUVM_FALSE_TUPLE TUUVM_TUPLE_MAKE_IMMEDIATE_TRIVIAL_WITH_INDEX(TUUVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_FALSE)
#define TUUVM_TRUE_TUPLE TUUVM_TUPLE_MAKE_IMMEDIATE_TRIVIAL_WITH_INDEX(TUUVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_TRUE)
#define TUUVM_VOID_TUPLE TUUVM_TUPLE_MAKE_IMMEDIATE_TRIVIAL_WITH_INDEX(TUUVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_VOID)
#define TUUVM_HASHTABLE_EMPTY_ELEMENT_TUPLE TUUVM_TUPLE_MAKE_IMMEDIATE_TRIVIAL_WITH_INDEX(TUUVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_HASHTABLE_EMPTY_ELEMENT)

#define TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(structureType) ((sizeof(structureType) - sizeof(tuuvm_tuple_header_t)) / sizeof(tuuvm_tuple_t))
#define TUUVM_BYTE_SIZE_FOR_STRUCTURE_TYPE(structureType) (sizeof(structureType) - sizeof(tuuvm_tuple_header_t))

#define TUUVM_STRING_PRINTF_FORMAT "%.*s"
#define TUUVM_STRING_PRINTF_ARG(arg) (int)tuuvm_tuple_getSizeInBytes(arg), TUUVM_CAST_OOP_TO_OBJECT_TUPLE(arg)->bytes

/**
 * Is this a pointer tuple?
 */
TUUVM_INLINE bool tuuvm_tuple_isPointer(tuuvm_tuple_t tuple)
{
    return (tuple & TUUVM_TUPLE_TAG_BIT_MASK) == TUUVM_TUPLE_TAG_POINTER;
}

/**
 * Is this non-null pointer tuple?
 */
TUUVM_INLINE bool tuuvm_tuple_isNonNullPointer(tuuvm_tuple_t tuple)
{
    return (tuple != 0) & ((tuple & TUUVM_TUPLE_TAG_BIT_MASK) == TUUVM_TUPLE_TAG_POINTER);
}

/**
 * Is this an immediate tuple?
 */
TUUVM_INLINE bool tuuvm_tuple_isImmediate(tuuvm_tuple_t tuple)
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
 * Gets the size in bytes of the specified tuple.
 */
TUUVM_INLINE size_t tuuvm_tuple_getSizeInSlots(tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isNonNullPointer(tuple) ? TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.objectSize / sizeof(tuuvm_tuple_t) : 0;
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
    return !tuuvm_tuple_isNonNullPointer(tuple) || (TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.typePointerAndFlags & TUUVM_TUPLE_NEEDS_FINALIZATION) != 0;
}

/**
 * Gets the GC color?
 */
TUUVM_INLINE uint32_t tuuvm_tuple_getGCColor(tuuvm_tuple_t tuple)
{
    return (uint32_t)(TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.typePointerAndFlags & TUUVM_TUPLE_GC_COLOR_MASK);
}

/**
 * Gets the GC color?
 */
TUUVM_INLINE void tuuvm_tuple_setGCColor(tuuvm_tuple_t tuple, uint32_t newColor)
{
    tuuvm_object_tuple_t *objectTuple = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple);
    objectTuple->header.typePointerAndFlags &= ~TUUVM_TUPLE_GC_COLOR_MASK;
    objectTuple->header.typePointerAndFlags |= newColor;
}

/**
 * Decodes a small integer as an immediate tuple.
 */
TUUVM_INLINE tuuvm_stuple_t tuuvm_tuple_integer_decodeSmall(tuuvm_tuple_t tuple)
{
    return ((tuuvm_stuple_t)tuple) >> TUUVM_TUPLE_TAG_BIT_COUNT;
}

/**
 * Encodes a small integer as an immediate tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_integer_encodeSmall(tuuvm_stuple_t value)
{
    return (((tuuvm_stuple_t)value) << TUUVM_TUPLE_TAG_BIT_COUNT) | TUUVM_TUPLE_TAG_INTEGER;
}

/**
 * Decodes an integer as an int32
 */
TUUVM_API int32_t tuuvm_tuple_integer_decodeInt32(tuuvm_context_t *context, tuuvm_tuple_t value);

/**
 * Decodes an integer as an uint32
 */
TUUVM_API uint32_t tuuvm_tuple_integer_decodeUInt32(tuuvm_context_t *context, tuuvm_tuple_t value);

/**
 * Decodes an integer as an int64
 */
TUUVM_API int64_t tuuvm_tuple_integer_decodeInt64(tuuvm_context_t *context, tuuvm_tuple_t value);

/**
 * Decodes an integer as an uint64
 */
TUUVM_API uint64_t tuuvm_tuple_integer_decodeUInt64(tuuvm_context_t *context, tuuvm_tuple_t value);

/**
 * Encodes an int32 as an integer.
 */
TUUVM_API tuuvm_tuple_t tuuvm_tuple_integer_encodeBigInt32(tuuvm_context_t *context, int32_t value);

/**
 * Encodes an uint32 as an integer.
 */
TUUVM_API tuuvm_tuple_t tuuvm_tuple_integer_encodeBigUInt32(tuuvm_context_t *context, uint32_t value);

/**
 * Encodes an int64 as an integer.
 */
TUUVM_API tuuvm_tuple_t tuuvm_tuple_integer_encodeBigInt64(tuuvm_context_t *context, int64_t value);

/**
 * Encodes an uint64 as an integer.
 */
TUUVM_API tuuvm_tuple_t tuuvm_tuple_integer_encodeBigUInt64(tuuvm_context_t *context, uint64_t value);

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

/**
 * Encodes an integer as a tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_integer_encodeUInt32(tuuvm_context_t *context, uint32_t value)
{
    if(sizeof(uint32_t) < sizeof(tuuvm_stuple_t) || (value <= TUUVM_IMMEDIATE_INT_MAX))
        return tuuvm_tuple_integer_encodeSmall((tuuvm_stuple_t)value);
    else
        return tuuvm_tuple_integer_encodeBigUInt32(context, value);
}

/**
 * Encodes an integer as a tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_integer_encodeInt32(tuuvm_context_t *context, int32_t value)
{
    if(sizeof(int32_t) < sizeof(tuuvm_stuple_t) || (TUUVM_IMMEDIATE_INT_MIN <= value && value <= TUUVM_IMMEDIATE_INT_MAX))
        return tuuvm_tuple_integer_encodeSmall((tuuvm_stuple_t)value);
    else
        return tuuvm_tuple_integer_encodeBigInt32(context, value);
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

/**
 * Encodes an integer as a tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_integer_encodeInt64(tuuvm_context_t *context, int64_t value)
{
    if(sizeof(int64_t) < sizeof(tuuvm_stuple_t) || (TUUVM_IMMEDIATE_INT_MIN <= value && value <= TUUVM_IMMEDIATE_INT_MAX))
        return tuuvm_tuple_integer_encodeSmall((tuuvm_stuple_t)value);
    else
        return tuuvm_tuple_integer_encodeBigInt64(context, value);
}

/**
 * Encodes an integer as a tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_integer_encodeUInt64(tuuvm_context_t *context, uint64_t value)
{
    if(sizeof(uint64_t) < sizeof(tuuvm_stuple_t) || value <= TUUVM_IMMEDIATE_INT_MAX)
        return tuuvm_tuple_integer_encodeSmall((tuuvm_stuple_t)value);
    else
        return tuuvm_tuple_integer_encodeBigUInt64(context, value);
}

/**
 * Encodes an integer as a tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_integer_decodeSize(tuuvm_context_t *context, size_t value)
{
    if(sizeof(uint32_t) == sizeof(size_t))
        return tuuvm_tuple_integer_decodeUInt32(context, value);
    else
        return tuuvm_tuple_integer_decodeUInt64(context, value);
}

/**
 * Encodes an integer as a tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_integer_encodeSize(tuuvm_context_t *context, size_t value)
{
    if(sizeof(uint32_t) == sizeof(size_t))
        return tuuvm_tuple_integer_encodeUInt32(context, (uint32_t)value);
    else
        return tuuvm_tuple_integer_encodeUInt64(context, value);
}

/**
 * Encodes a Char8 as an immediate tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_char8_encode(tuuvm_char8_t value)
{
    return (((tuuvm_tuple_t)value) << TUUVM_TUPLE_TAG_BIT_COUNT) | TUUVM_TUPLE_TAG_CHAR8;
}

/**
 * Decodes a Char from an immediate tuple.
 */
TUUVM_INLINE tuuvm_char8_t tuuvm_tuple_char8_decode(tuuvm_tuple_t tuple)
{
    return (tuuvm_char8_t)(tuple >> TUUVM_TUPLE_TAG_BIT_COUNT);
}

/**
 * Encodes an UInt8 as an immediate tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_uint8_encode(uint8_t value)
{
    return ((tuuvm_tuple_t)value << TUUVM_TUPLE_TAG_BIT_COUNT) | TUUVM_TUPLE_TAG_UINT8;
}

/**
 * Decodes an UInt8 from an immediate tuple.
 */
TUUVM_INLINE uint8_t tuuvm_tuple_uint8_decode(tuuvm_tuple_t tuple)
{
    return (uint8_t) (tuple >> TUUVM_TUPLE_TAG_BIT_COUNT);
}

/**
 * Encodes an Int8 as an immediate tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_int8_encode(int8_t value)
{
    return ((tuuvm_stuple_t)value << TUUVM_TUPLE_TAG_BIT_COUNT) | TUUVM_TUPLE_TAG_INT8;
}

/**
 * Decodes an Int8 from an immediate tuple.
 */
TUUVM_INLINE int8_t tuuvm_tuple_int8_decode(tuuvm_tuple_t tuple)
{
    return (int8_t) ((tuuvm_stuple_t)tuple >> TUUVM_TUPLE_TAG_BIT_COUNT);
}

/**
 * Encodes a Char16 as an immediate tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_char16_encode(tuuvm_char16_t value)
{
    return (((tuuvm_tuple_t)value) << TUUVM_TUPLE_TAG_BIT_COUNT) | TUUVM_TUPLE_TAG_CHAR16;
}

/**
 * Decodes a Char from an immediate tuple.
 */
TUUVM_INLINE tuuvm_char16_t tuuvm_tuple_char16_decode(tuuvm_tuple_t tuple)
{
    return (tuuvm_char16_t)(tuple >> TUUVM_TUPLE_TAG_BIT_COUNT);
}

/**
 * Encodes an UInt16 as an immediate tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_uint16_encode(uint16_t value)
{
    return ((tuuvm_tuple_t)value << TUUVM_TUPLE_TAG_BIT_COUNT) | TUUVM_TUPLE_TAG_UINT16;
}

/**
 * Decodes an UInt16 from an immediate tuple.
 */
TUUVM_INLINE uint16_t tuuvm_tuple_uint16_decode(tuuvm_tuple_t tuple)
{
    return (uint16_t) (tuple >> TUUVM_TUPLE_TAG_BIT_COUNT);
}

/**
 * Encodes an Int16 as an immediate tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_int16_encode(int16_t value)
{
    return ((tuuvm_stuple_t)value << TUUVM_TUPLE_TAG_BIT_COUNT) | TUUVM_TUPLE_TAG_INT16;
}

/**
 * Decodes an Int16 from an immediate tuple.
 */
TUUVM_INLINE int16_t tuuvm_tuple_int16_decode(tuuvm_tuple_t tuple)
{
    return (int16_t) ((tuuvm_stuple_t)tuple >> TUUVM_TUPLE_TAG_BIT_COUNT);
}

/**
 * Encodes a Char32 as an immediate tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_char32_encodeSmall(tuuvm_char32_t value)
{
    return (((tuuvm_tuple_t)value) << TUUVM_TUPLE_TAG_BIT_COUNT) | TUUVM_TUPLE_TAG_CHAR32;
}

/**
 * Encodes a Char32 as a tuple.
 */
TUUVM_API tuuvm_tuple_t tuuvm_tuple_char32_encodeBig(tuuvm_context_t *context, tuuvm_char32_t value);

/**
 * Decodes a Char from an immediate tuple.
 */
TUUVM_INLINE tuuvm_char32_t tuuvm_tuple_char32_decodeSmall(tuuvm_tuple_t tuple)
{
    return (tuuvm_char32_t)(tuple >> TUUVM_TUPLE_TAG_BIT_COUNT);
}

/**
 * Decodes a Char32 from a tuple.
 */
TUUVM_INLINE tuuvm_char32_t tuuvm_tuple_char32_decode(tuuvm_tuple_t tuple)
{
    if(tuuvm_tuple_isImmediate(tuple))
        return tuuvm_tuple_char32_decodeSmall(tuple);
    return *((tuuvm_char32_t*)TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->bytes);
}

/**
 * Encodes an UInt32 as an immediate tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_uint32_encodeSmall(uint32_t value)
{
    return ((tuuvm_tuple_t)value << TUUVM_TUPLE_TAG_BIT_COUNT) | TUUVM_TUPLE_TAG_UINT32;
}

/*
 * Encodes an UInt32 as a bytes tuple.
 */
TUUVM_API tuuvm_tuple_t tuuvm_tuple_uint32_encodeBig(tuuvm_context_t *context, uint32_t value);

/**
 * Decodes an UInt32 from an immediate tuple.
 */
TUUVM_INLINE uint32_t tuuvm_tuple_uint32_decodeSmall(tuuvm_tuple_t tuple)
{
    return (uint32_t) (tuple >> TUUVM_TUPLE_TAG_BIT_COUNT);
}

/*
 * Decodes an UInt32 from a tuple.
 */
TUUVM_INLINE uint32_t tuuvm_tuple_uint32_decode(tuuvm_tuple_t tuple)
{
    if(tuuvm_tuple_isImmediate(tuple) || !tuple)
        return tuuvm_tuple_uint32_decodeSmall(tuple);
    return *((uint32_t*)TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->bytes);
}

/**
 * Encodes an Int32 as an immediate tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_int32_encodeSmall(int32_t value)
{
    return ((tuuvm_stuple_t)value << TUUVM_TUPLE_TAG_BIT_COUNT) | TUUVM_TUPLE_TAG_INT32;
}

/**
 * Encodes an Int32 as a bytes tuple.
 */
TUUVM_API tuuvm_tuple_t tuuvm_tuple_int32_encodeBig(tuuvm_context_t *context, int32_t value);

/**
 * Decodes an Int32 from an immediate tuple.
 */
TUUVM_INLINE int32_t tuuvm_tuple_int32_decodeSmall(tuuvm_tuple_t tuple)
{
    return (int32_t) ((tuuvm_stuple_t)tuple >> TUUVM_TUPLE_TAG_BIT_COUNT);
}

/**
 * Decodes an Int32 from a tuple.
 */
TUUVM_INLINE int32_t tuuvm_tuple_int32_decode(tuuvm_tuple_t tuple)
{
    if(tuuvm_tuple_isImmediate(tuple))
        return tuuvm_tuple_int32_decodeSmall(tuple);
    return *((int32_t*)TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->bytes);
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

/**
 * Encodes a Char32 as a tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_char32_encode(tuuvm_context_t *context, tuuvm_char32_t value)
{
    if(sizeof(tuuvm_char32_t) < sizeof(tuuvm_tuple_t) || value <= TUUVM_IMMEDIATE_UINT_MAX)
        return tuuvm_tuple_char32_encodeSmall(value);
    else
        return tuuvm_tuple_char32_encodeBig(context, value);
}

/**
 * Encodes an UInt32 as a tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_uint32_encode(tuuvm_context_t *context, uint32_t value)
{
    if(sizeof(uint32_t) < sizeof(tuuvm_tuple_t) || value <= TUUVM_IMMEDIATE_UINT_MAX)
        return tuuvm_tuple_uint32_encodeSmall(value);
    else
        return tuuvm_tuple_uint32_encodeBig(context, value);
}

/**
 * Encodes an Int32 as a tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_int32_encode(tuuvm_context_t *context, int32_t value)
{
    if(sizeof(int32_t) < sizeof(tuuvm_tuple_t) || (TUUVM_IMMEDIATE_INT_MIN <= value && value <= TUUVM_IMMEDIATE_INT_MAX))
        return tuuvm_tuple_int32_encodeSmall(value);
    else
        return tuuvm_tuple_int32_encodeBig(context, value);
}

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

/**
 * Encodes an UInt64 as an immediate tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_uint64_encodeSmall(uint64_t value)
{
    return ((tuuvm_tuple_t)value << TUUVM_TUPLE_TAG_BIT_COUNT) | TUUVM_TUPLE_TAG_UINT64;
}

/*
 * Encodes an UInt64 as a bytes tuple.
 */
TUUVM_API tuuvm_tuple_t tuuvm_tuple_uint64_encodeBig(tuuvm_context_t *context, uint64_t value);

/**
 * Decodes an UInt64 from an immediate tuple.
 */
TUUVM_INLINE uint64_t tuuvm_tuple_uint64_decodeSmall(tuuvm_tuple_t tuple)
{
    return (uint64_t) (tuple >> TUUVM_TUPLE_TAG_BIT_COUNT);
}

/*
 * Decodes an UInt64 from a tuple.
 */
TUUVM_INLINE uint64_t tuuvm_tuple_uint64_decode(tuuvm_tuple_t tuple)
{
    if(tuuvm_tuple_isImmediate(tuple) || !tuple)
        return tuuvm_tuple_uint64_decodeSmall(tuple);
    return *((uint64_t*)TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->bytes);
}

/**
 * Encodes an Int64 as an immediate tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_int64_encodeSmall(int64_t value)
{
    return ((tuuvm_stuple_t)value << TUUVM_TUPLE_TAG_BIT_COUNT) | TUUVM_TUPLE_TAG_INT64;
}

/*
 * Encodes an Int64 as a bytes tuple.
 */
TUUVM_API tuuvm_tuple_t tuuvm_tuple_int64_encodeBig(tuuvm_context_t *context, int64_t value);

/**
 * Decodes an Int64 from an immediate tuple.
 */
TUUVM_INLINE int64_t tuuvm_tuple_int64_decodeSmall(tuuvm_tuple_t tuple)
{
    return (int64_t) ((tuuvm_stuple_t)tuple >> TUUVM_TUPLE_TAG_BIT_COUNT);
}

/**
 * Decodes an Int64 from a tuple.
 */
TUUVM_INLINE int64_t tuuvm_tuple_int64_decode(tuuvm_tuple_t tuple)
{
    if(tuuvm_tuple_isImmediate(tuple))
        return tuuvm_tuple_int64_decodeSmall(tuple);
    return *((int64_t*)TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->bytes);
}

/**
 * Encodes an UInt64 as a tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_uint64_encode(tuuvm_context_t *context, uint64_t value)
{
    if(sizeof(uint64_t) < sizeof(tuuvm_tuple_t) || value <= TUUVM_IMMEDIATE_UINT_MAX)
        return tuuvm_tuple_uint64_encodeSmall(value);
    else
        return tuuvm_tuple_uint64_encodeBig(context, value);
}

/**
 * Encodes an Int64 as a tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_int64_encode(tuuvm_context_t *context, int64_t value)
{
    if(sizeof(int64_t) < sizeof(tuuvm_tuple_t) || (TUUVM_IMMEDIATE_INT_MIN <= value && value <= TUUVM_IMMEDIATE_INT_MAX))
        return tuuvm_tuple_int64_encodeSmall(value);
    else
        return tuuvm_tuple_int64_encodeBig(context, value);
}

/**
 * Encodes a size as a tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_size_encode(tuuvm_context_t *context, size_t value)
{
    if(sizeof(size_t) == sizeof(uint32_t))
        return tuuvm_tuple_uint32_encode(context, (uint32_t)value);
    else
        return tuuvm_tuple_uint64_encode(context, (uint64_t)value);
}

/**
 * Decodes a size from tuple.
 */
TUUVM_INLINE size_t tuuvm_tuple_size_decode(tuuvm_tuple_t tuple)
{
    if(sizeof(size_t) == sizeof(uint32_t))
        return tuuvm_tuple_uint32_decode(tuple);
    else
        return tuuvm_tuple_uint64_decode(tuple);
}

/**
 * Encodes an uintptr_t as a tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_uintptr_encode(tuuvm_context_t *context, uintptr_t value)
{
    if(sizeof(uintptr_t) == sizeof(uint32_t))
        return tuuvm_tuple_uint32_encode(context, (uint32_t)value);
    else
        return tuuvm_tuple_uint64_encode(context, (uint64_t)value);
}

/**
 * Decodes a intptr_t from tuple.
 */
TUUVM_INLINE uintptr_t tuuvm_tuple_uintptr_decode(tuuvm_tuple_t tuple)
{
    if(sizeof(uintptr_t) == sizeof(uint32_t))
        return tuuvm_tuple_uint32_decode(tuple);
    else
        return tuuvm_tuple_uint64_decode(tuple);
}

/**
 * Encodes an intptr_t as a tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_intptr_encode(tuuvm_context_t *context, intptr_t value)
{
    if(sizeof(intptr_t) == sizeof(int32_t))
        return tuuvm_tuple_int32_encode(context, (int32_t)value);
    else
        return tuuvm_tuple_int64_encode(context, (int64_t)value);
}

/**
 * Decodes a intptr_t from tuple.
 */
TUUVM_INLINE intptr_t tuuvm_tuple_intptr_decode(tuuvm_tuple_t tuple)
{
    if(sizeof(intptr_t) == sizeof(int32_t))
        return tuuvm_tuple_int32_decode(tuple);
    else
        return tuuvm_tuple_int64_decode(tuple);
}

/**
 * Decodes a float32 from a tuple.
 */
TUUVM_API tuuvm_float32_t tuuvm_tuple_float32_decode(tuuvm_tuple_t tuple);

/**
 * Encodes a float32 as a tuple.
 */
TUUVM_API tuuvm_tuple_t tuuvm_tuple_float32_encode(tuuvm_context_t *context, tuuvm_float32_t value);

/**
 * Decodes a float64 from a tuple.
 */
TUUVM_API tuuvm_float64_t tuuvm_tuple_float64_decode(tuuvm_tuple_t tuple);

/**
 * Encodes a float64 as a tuple.
 */
TUUVM_API tuuvm_tuple_t tuuvm_tuple_float64_encode(tuuvm_context_t *context, tuuvm_float64_t value);

/**
 * Encodes a boolean value.
 */ 
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_boolean_encode(bool value)
{
    return value ? TUUVM_TRUE_TUPLE : TUUVM_FALSE_TUPLE;
}

/**
 * Encodes a boolean value.
 */ 
TUUVM_INLINE bool tuuvm_tuple_boolean_decode(tuuvm_tuple_t value)
{
    return value == TUUVM_TRUE_TUPLE;
}

/**
 * Computes or retrieves the identity hash 
 */
TUUVM_INLINE size_t tuuvm_tuple_identityHash(tuuvm_tuple_t tuple)
{
    return (tuuvm_tuple_isNonNullPointer(tuple) ? TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.identityHash : tuuvm_hashMultiply(tuple)) & TUUVM_TUPLE_IMMEDIATE_VALUE_BIT_MASK;
}

/**
 * Compares two tuples for identity equality.
 */
TUUVM_INLINE bool tuuvm_tuple_identityEquals(tuuvm_tuple_t a, tuuvm_tuple_t b)
{
    return a == b;
}

/**
 * Compares two tuples for identity inequality.
 */
TUUVM_INLINE bool tuuvm_tuple_identityNotEquals(tuuvm_tuple_t a, tuuvm_tuple_t b)
{
    return a != b;
}

/**
 * Decodes any size valuable tuple.
 */
TUUVM_INLINE size_t tuuvm_tuple_anySize_decode(tuuvm_tuple_t tuple)
{
    if(tuuvm_tuple_isImmediate(tuple))
        return (size_t)(tuple>>TUUVM_TUPLE_TAG_BIT_COUNT);
    return 0;
}

/**
 * The primitive identity hash function.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_tuple_primitive_identityHash(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments);

/**
 * The primitive identity equals function.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_tuple_primitive_identityEquals(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments);;

/**
 * The primitive identity not-equals function.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_tuple_primitive_identityNotEquals(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments);;

/**
 * Makes a C string with the specified bytes tuple.
 */
TUUVM_API char *tuuvm_tuple_bytesToCString(tuuvm_tuple_t tuple);

/**
 * Frees a C string that was allocated for converting a tuple into a C string.
 */
TUUVM_API void tuuvm_tuple_bytesToCStringFree(char *cstring);

/**
 * Gets the value of a slot in a tuple
 */
TUUVM_API tuuvm_tuple_t tuuvm_tuple_slotAt(tuuvm_context_t *context, tuuvm_tuple_t tuple, size_t slotIndex);

/**
 * Sets the value in a slot of a tuple
 */
TUUVM_API void tuuvm_tuple_slotAtPut(tuuvm_context_t *context, tuuvm_tuple_t tuple, size_t slotIndex, tuuvm_tuple_t value);

/**
 * Is this tuple a kind of the specified type?
 */
TUUVM_API bool tuuvm_tuple_isKindOf(tuuvm_context_t *context, tuuvm_tuple_t tuple, tuuvm_tuple_t type);

#endif //TUUVM_TUPLE_H

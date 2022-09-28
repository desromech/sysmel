#ifndef TUUVM_TUPLE_H
#define TUUVM_TUPLE_H

#pragma once

#include "common.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct tuuvm_context_s tuuvm_context_t;

typedef uint8_t tuuvm_char8_t;
typedef uint16_t tuuvm_char16_t;
typedef uint32_t tuuvm_char32_t;

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

#define TUUVM_TUPLE_GC_COLOR_MASK ((uintptr_t)-4)
#define TUUVM_TUPLE_BYTES_BIT ((uintptr_t)4)
#define TUUVM_TUPLE_IMMUTABLE_BIT ((uintptr_t)8)

#define TUUVM_TUPLE_TAG_BIT_COUNT 4
#define TUUVM_TUPLE_TAG_BIT_MASK 15

#define TUUVM_CAST_OOP_TO_OBJECT_TUPLE(oop) ((tuuvm_object_tuple_t*)oop)

enum
{
    TUUVM_TUPLE_BIT_COUNT = sizeof(tuuvm_tuple_t)*8,
    TUUVM_IMMEDIATE_BIT_COUNT = TUUVM_TUPLE_BIT_COUNT - TUUVM_TUPLE_TAG_BIT_COUNT,

    TUUVM_IMMEDIATE_UINT_MIN = 0,
    TUUVM_IMMEDIATE_UINT_MAX = ((tuuvm_tuple_t)1 << TUUVM_IMMEDIATE_BIT_COUNT) - 1,

    TUUVM_IMMEDIATE_INT_MIN = -((tuuvm_stuple_t)1 << (TUUVM_IMMEDIATE_BIT_COUNT - 1)),
    TUUVM_IMMEDIATE_INT_MAX = ((tuuvm_stuple_t)1 << (TUUVM_IMMEDIATE_BIT_COUNT - 1)) - 1,
};

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

#define TUUVM_TUPLE_MAKE_IMMEDIATE_TRIVIAL_WITH_INDEX(immediateTrivialIndex) ((immediateTrivialIndex << TUUVM_TUPLE_TAG_BIT_COUNT) | TUUVM_TUPLE_TAG_TRIVIAL)
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
#define TUUVM_FALSE_TUPLE TUUVM_TUPLE_MAKE_IMMEDIATE_TRIVIAL_WITH_INDEX(TUUVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_FALSE)
#define TUUVM_TRUE_TUPLE TUUVM_TUPLE_MAKE_IMMEDIATE_TRIVIAL_WITH_INDEX(TUUVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_TRUE)
#define TUUVM_VOID_TUPLE TUUVM_TUPLE_MAKE_IMMEDIATE_TRIVIAL_WITH_INDEX(TUUVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_VOID)

#define TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(structureType) ((sizeof(structureType) - sizeof(tuuvm_tuple_header_t)) / sizeof(tuuvm_tuple_t))
#define TUUVM_BYTE_SIZE_FOR_STRUCTURE_TYPE(structureType) (sizeof(structureType) - sizeof(tuuvm_tuple_header_t))

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

/*
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

/*
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
    if(tuuvm_tuple_isImmediate(tuple))
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

/*
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

/*
 * Decodes an Int32 from a tuple.
 */
TUUVM_INLINE int32_t tuuvm_tuple_int32_decode(tuuvm_tuple_t tuple)
{
    if(tuuvm_tuple_isImmediate(tuple))
        return tuuvm_tuple_int32_decodeSmall(tuple);
    return *((int32_t*)TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->bytes);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"

/*
 * Encodes a Char32 as a tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_char32_encode(tuuvm_context_t *context, tuuvm_char32_t value)
{
    if(sizeof(tuuvm_char32_t) < sizeof(tuuvm_tuple_t) || value <= TUUVM_IMMEDIATE_UINT_MAX)
        return tuuvm_tuple_char32_encodeSmall(value);
    else
        return tuuvm_tuple_char32_encodeBig(context, value);
}

/*
 * Encodes an UInt32 as a tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_uint32_encode(tuuvm_context_t *context, uint32_t value)
{
    if(sizeof(uint32_t) < sizeof(tuuvm_tuple_t) || value <= TUUVM_IMMEDIATE_UINT_MAX)
        return tuuvm_tuple_uint32_encodeSmall(value);
    else
        return tuuvm_tuple_uint32_encodeBig(context, value);
}

/*
 * Encodes an Int32 as a tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_int32_encode(tuuvm_context_t *context, int32_t value)
{
    if(sizeof(int32_t) < sizeof(tuuvm_tuple_t) || (TUUVM_IMMEDIATE_INT_MIN <= value && value <= TUUVM_IMMEDIATE_INT_MAX))
        return tuuvm_tuple_int32_encodeSmall(value);
    else
        return tuuvm_tuple_int32_encodeBig(context, value);
}

#pragma GCC diagnostic pop

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
    if(tuuvm_tuple_isImmediate(tuple))
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

/*
 * Decodes an Int64 from a tuple.
 */
TUUVM_INLINE int64_t tuuvm_tuple_int64_decode(tuuvm_tuple_t tuple)
{
    if(tuuvm_tuple_isImmediate(tuple))
        return tuuvm_tuple_int64_decodeSmall(tuple);
    return *((int64_t*)TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->bytes);
}

/*
 * Encodes an UInt64 as a tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_uint64_encode(tuuvm_context_t *context, uint64_t value)
{
    if(sizeof(uint64_t) < sizeof(tuuvm_tuple_t) || value <= TUUVM_IMMEDIATE_UINT_MAX)
        return tuuvm_tuple_uint64_encodeSmall(value);
    else
        return tuuvm_tuple_uint64_encodeBig(context, value);
}

/*
 * Encodes an Int64 as a tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_int64_encode(tuuvm_context_t *context, int64_t value)
{
    if(sizeof(int64_t) < sizeof(tuuvm_tuple_t) || (TUUVM_IMMEDIATE_INT_MIN <= value && value <= TUUVM_IMMEDIATE_INT_MAX))
        return tuuvm_tuple_int64_encodeSmall(value);
    else
        return tuuvm_tuple_int64_encodeBig(context, value);
}

/*
 * Encodes a size as a tuple.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_size_encode(tuuvm_context_t *context, size_t value)
{
    if(sizeof(size_t) == sizeof(uint32_t))
        return tuuvm_tuple_uint32_encode(context, (uint32_t)value);
    else
        return tuuvm_tuple_uint64_encode(context, (uint32_t)value);
}

/*
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
 * Encodes a boolean value.
 */ 
TUUVM_INLINE tuuvm_tuple_t tuuvm_tuple_boolean_encode(bool value)
{
    return value ? TUUVM_TRUE_TUPLE : TUUVM_FALSE_TUPLE;
}

#endif //TUUVM_TUPLE_H

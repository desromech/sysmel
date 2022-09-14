#ifndef TUUVM_TUPLE_H
#define TUUVM_TUPLE_H

#pragma once

#include "common.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define TUUVM_TUPLE_TYPE_POINTER_MASK ((uintptr_t)-16)
#define TUUVM_TUPLE_FLAGS_MASK ((uintptr_t)15)

#define TUUVM_TUPLE_GC_COLOR_MASK ((uintptr_t)-4)
#define TUUVM_TUPLE_BYTES_BIT ((uintptr_t)4)
#define TUUVM_TUPLE_IMMUTABLE_BIT ((uintptr_t)8)

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
typedef struct tuvvm_tuple_s
{
    tuuvm_tuple_header_t header;

    union
    {
        struct tuvvm_tuple_s *pointers[1];
        uint8_t bytes[1];
    };
} tuvvm_tuple_t;

#define TUUVM_NULL_TUPLE ((tuvvm_tuple_t*)0)

/**
 * Gets the size in bytes of the specified tuple.
 */
static inline size_t tuuvm_tuple_getSizeInBytes(tuvvm_tuple_t *tuple)
{
    return tuple->header.objectSize;
}

/**
 * Gets the type tuple of the specified tuple.
 */
static inline tuvvm_tuple_t *tuuvm_tuple_getType(tuvvm_tuple_t *tuple)
{
    return (tuvvm_tuple_t*)(tuple->header.typePointerAndFlags & TUUVM_TUPLE_TYPE_POINTER_MASK);
}

/**
 * Is this a bytes tuple?
 */
static inline bool tuuvm_tuple_isBytes(tuvvm_tuple_t *tuple)
{
    return (tuple->header.typePointerAndFlags & TUUVM_TUPLE_BYTES_BIT) != 0;
}

/**
 * Is this an immutable tuple?
 */
static inline bool tuuvm_tuple_isImmutable(tuvvm_tuple_t *tuple)
{
    return (tuple->header.typePointerAndFlags & TUUVM_TUPLE_IMMUTABLE_BIT) != 0;
}

#endif //TUUVM_TUPLE_H

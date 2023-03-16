#ifndef TUUVM_HASH_H
#define TUUVM_HASH_H

#pragma once

#include "common.h"
#include <stddef.h>
#include <stdint.h>

#define TUUVM_HASH_BIT_COUNT (sizeof(uintptr_t)*8 - 4)
#define TUUVM_HASH_BIT_MASK (((uintptr_t)1 << TUUVM_HASH_BIT_COUNT) - 1)

TUUVM_INLINE size_t tuuvm_hashMultiply(size_t hash)
{
    return (hash * (size_t)1103515245) & TUUVM_HASH_BIT_MASK;
}

TUUVM_INLINE size_t tuuvm_hashConcatenate(size_t previousHash, size_t nextHash)
{
    return (tuuvm_hashMultiply(previousHash) + nextHash) & TUUVM_HASH_BIT_MASK;
}

#endif //TUUVM_HASH_H
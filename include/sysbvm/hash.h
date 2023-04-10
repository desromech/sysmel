#ifndef SYSBVM_HASH_H
#define SYSBVM_HASH_H

#pragma once

#include "common.h"
#include <stddef.h>
#include <stdint.h>

#define SYSBVM_HASH_BIT_COUNT (sizeof(uintptr_t)*8 - 4)
#define SYSBVM_HASH_BIT_MASK (((uintptr_t)1 << SYSBVM_HASH_BIT_COUNT) - 1)

SYSBVM_INLINE size_t sysbvm_hashMultiply(size_t hash)
{
    return (hash * (size_t)1103515245) & SYSBVM_HASH_BIT_MASK;
}

SYSBVM_INLINE size_t sysbvm_hashConcatenate(size_t previousHash, size_t nextHash)
{
    return (sysbvm_hashMultiply(previousHash) + nextHash) & SYSBVM_HASH_BIT_MASK;
}

#endif //SYSBVM_HASH_H
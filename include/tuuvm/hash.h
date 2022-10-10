#ifndef TUUVM_HASH_H
#define TUUVM_HASH_H

#include "common.h"
#include <stddef.h>

TUUVM_INLINE size_t tuuvm_hashMultiply(size_t hash)
{
    return hash * (size_t)1103515245;
}

#endif //TUUVM_HASH_H
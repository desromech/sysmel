#ifndef TUUVM_INTERNAL_CONTEXT_H
#define TUUVM_INTERNAL_CONTEXT_H

#pragma once

#include "tuuvm/context.h"
#include "heap.h"

typedef struct tuuvm_context_roots_s
{
    tuuvm_tuple_t immediateTypeTable[TUUVM_TUPLE_TAG_COUNT];
    tuuvm_tuple_t immediateTrivialTypeTable[TUUVM_TUPLE_IMMEDIATE_TRIVIAL_COUNT];
} tuuvm_context_roots_t;

struct tuuvm_context_s
{
    tuuvm_heap_t heap;
    tuuvm_context_roots_t roots;
};

#endif //TUUVM_INTERNAL_CONTEXT_H

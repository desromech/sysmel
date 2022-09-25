#ifndef TUUVM_INTERNAL_CONTEXT_H
#define TUUVM_INTERNAL_CONTEXT_H

#pragma once

#include "tuuvm/context.h"
#include "heap.h"

typedef struct tuuvm_context_roots_s
{
    tuuvm_tuple_t immediateTypeTable[TUUVM_TUPLE_TAG_COUNT];
    tuuvm_tuple_t immediateTrivialTypeTable[TUUVM_TUPLE_IMMEDIATE_TRIVIAL_COUNT];

    tuuvm_tuple_t arrayType;
    tuuvm_tuple_t arrayListType;
    tuuvm_tuple_t setType;
    tuuvm_tuple_t stringType;
    tuuvm_tuple_t symbolType;
    tuuvm_tuple_t typeType;
} tuuvm_context_roots_t;

struct tuuvm_context_s
{
    tuuvm_heap_t heap;
    tuuvm_context_roots_t roots;
};

#endif //TUUVM_INTERNAL_CONTEXT_H

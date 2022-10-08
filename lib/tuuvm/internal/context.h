#ifndef TUUVM_INTERNAL_CONTEXT_H
#define TUUVM_INTERNAL_CONTEXT_H

#pragma once

#include "tuuvm/context.h"
#include "heap.h"

typedef struct tuuvm_context_roots_s
{
    tuuvm_tuple_t immediateTypeTable[TUUVM_TUPLE_TAG_COUNT];
    tuuvm_tuple_t immediateTrivialTypeTable[TUUVM_TUPLE_IMMEDIATE_TRIVIAL_COUNT];
    tuuvm_tuple_t internedSymbolSet;

    tuuvm_tuple_t arrayType;
    tuuvm_tuple_t arraySliceType;
    tuuvm_tuple_t arrayListType;
    tuuvm_tuple_t byteArrayType;
    tuuvm_tuple_t falseType;
    tuuvm_tuple_t hashtableEmptyType;
    tuuvm_tuple_t integerType;
    tuuvm_tuple_t primitiveFunctionType;
    tuuvm_tuple_t setType;
    tuuvm_tuple_t stringType;
    tuuvm_tuple_t symbolType;
    tuuvm_tuple_t trueType;
    tuuvm_tuple_t typeType;
    tuuvm_tuple_t nilType;
    tuuvm_tuple_t voidType;

    tuuvm_tuple_t char8Type;
    tuuvm_tuple_t uint8Type;
    tuuvm_tuple_t int8Type;

    tuuvm_tuple_t char16Type;
    tuuvm_tuple_t uint16Type;
    tuuvm_tuple_t int16Type;

    tuuvm_tuple_t char32Type;
    tuuvm_tuple_t uint32Type;
    tuuvm_tuple_t int32Type;

    tuuvm_tuple_t uint64Type;
    tuuvm_tuple_t int64Type;

    tuuvm_tuple_t floatType;
    tuuvm_tuple_t doubleType;
} tuuvm_context_roots_t;

struct tuuvm_context_s
{
    tuuvm_heap_t heap;
    tuuvm_context_roots_t roots;
};

#endif //TUUVM_INTERNAL_CONTEXT_H

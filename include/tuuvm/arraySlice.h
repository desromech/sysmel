#ifndef TUUVM_ARRAY_SLICE_H
#define TUUVM_ARRAY_SLICE_H

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_type_arrayList_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t elements;
    tuuvm_tuple_t size;
} tuuvm_type_arrayList_t;

#endif //TUUVM_ARRAY_SLICE_H
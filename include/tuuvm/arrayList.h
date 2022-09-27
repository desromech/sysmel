#ifndef TUUVM_ARRAY_LIST_H
#define TUUVM_ARRAY_LIST_H

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_type_arrayList_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t size;
    tuuvm_tuple_t storage;
} tuuvm_type_arrayList_t;

#endif //TUUVM_ARRAY_LIST_H
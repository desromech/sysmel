#ifndef TUUVM_SET_H
#define TUUVM_SET_H

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_set_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t size;
    tuuvm_tuple_t storage;
    tuuvm_tuple_t equalsFunction;
    tuuvm_tuple_t hashFunction;
} tuuvm_set_t;

TUUVM_API tuuvm_tuple_t tuuvm_set_create(tuuvm_context_t *context, tuuvm_tuple_t equalsFunction, tuuvm_tuple_t hashFunction);

#endif //TUUVM_SET_H
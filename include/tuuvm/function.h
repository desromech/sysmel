#ifndef TUUVM_FUNCTION_H
#define TUUVM_FUNCTION_H

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef tuuvm_tuple_t (*tuuvm_functionEntryPoint_t)(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments);

typedef struct tuuvm_primitiveFunction_s
{
    tuuvm_tuple_header_t header;
    void *userdata;
    tuuvm_functionEntryPoint_t entryPoint;
} tuuvm_primitiveFunction_t;

TUUVM_API tuuvm_tuple_t tuuvm_function_createPrimitive(tuuvm_context_t *context, void *userdata, tuuvm_functionEntryPoint_t entryPoint);

#endif //TUUVM_FUNCTION_H
